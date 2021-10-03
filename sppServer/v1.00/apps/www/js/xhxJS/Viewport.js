/**
 * @author mrdoob / http://mrdoob.com/
 */

var Viewport = function ( editor ) {

	var signals = editor.signals;

	var xhxDoc = editor.xhxDoc;
	var xhxPartSurfaceActor = undefined;
	var xhxIsoSurfaceActor = undefined;
	var xhxCuttingPlaneActor = undefined;
	var xhxGlyphActor = undefined;
	var xhxStreamlineActor = undefined;
	var xhxColorBar = new XHX.ColorBar();
	var xhxResultTexture = undefined; //the texture used to display results, rainbow by default.
	var xhxCuttingPlaneMesh = undefined;//remember this so that the translation control can be enabled after disabled.
	
	var xhxDialog = undefined;//the visible dialog. any time, only one dialog can be seen.

	var xhxGnomeActor = new GnomeActor();
	var xhxGnomeRenderer = new THREE.WebGLRenderer( { antialias: true } );

	var xhxPicker = null; //built after xhxWarpCtrl. Right now it is for warp only.

	function updateGlyphDisplay() {
		if (xhxGlyphActor) {
			xhxGlyphActor.traverse(function(element) {
				if (element.name == "glyph_instances_mesh") {
					var study = xhxDoc.study;

					var planeABCD = xhxCuttingPlaneActor.plane;//a Vector4
					var lCutHalfWidth = xhxCuttingPlaneActor.cutHalfWidthScaleFactor * (study.data.size * 0.0075); //for glyph on cutting plane
			
					var geo = element.geometry;//glyph position and visibility are stored as geometry attributes.
					var gVisibility = geo.attributes.visibility;
					var mtx3 = geo.attributes.aInstanceMatrix3;
					for (var idx = 0; idx < gVisibility.count; idx++) {
						//last row is the translation part
						var posX = mtx3.getX(idx);
						var posY = mtx3.getY(idx);
						var posZ = mtx3.getZ(idx);

						var distance = posX * planeABCD.x + posY * planeABCD.y + posZ * planeABCD.z - planeABCD.w;
						if (xhxCuttingPlaneActor.glyphOnCutBothSides) {
							if (distance > lCutHalfWidth || distance < -lCutHalfWidth)
								gVisibility.setX(idx, 0); //not visible
							else
								gVisibility.setX(idx, 1);
						} else {
							if (distance > lCutHalfWidth)
								gVisibility.setX(idx, 0);
							else
								gVisibility.setX(idx, 1);
						}
					}
					gVisibility.needsUpdate = true;
				}
			});
		}

	}

	var container = new UI.Panel();
	container.setId( 'viewport' );
	container.setPosition( 'absolute' );

	//

	var renderer = null;

	var camera = editor.camera;
	var scene = editor.scene;
	var sceneHelpers = editor.sceneHelpers;

	var objects = [];

	// helpers

	var grid = new THREE.GridHelper( 30, 30, 0x444444, 0x888888 );
	sceneHelpers.add( grid );

	var array = grid.geometry.attributes.color.array;

	for ( var i = 0; i < array.length; i += 60 ) {

		for ( var j = 0; j < 12; j ++ ) {

			array[ i + j ] = 0.26;

		}

	}

	//

	var box = new THREE.Box3();

	var selectionBox = new THREE.BoxHelper();
	selectionBox.material.depthTest = false;
	selectionBox.material.transparent = true;
	selectionBox.visible = false;
	sceneHelpers.add( selectionBox );

	var objectPositionOnDown = null;
	var objectRotationOnDown = null;
	var objectScaleOnDown = null;

	var transformControls = new THREE.TransformControls( camera, container.dom );
	transformControls.setSpace( 'local' );
	transformControls.addEventListener( 'change', function () {

		var object = transformControls.object;

		if ( object !== undefined ) {

			selectionBox.setFromObject( object );

			if ( editor.helpers[ object.id ] !== undefined ) {

				editor.helpers[ object.id ].update();

			}

			if (object.geometry && object.geometry.type === 'PlaneBufferGeometry') {
				var study = xhxDoc.study;
				//cutting plane has been created or changed
				var lPlaneOrigin = object.position;
				var lPlaneNormal = new THREE.Vector3();
				lPlaneNormal.setFromMatrixColumn(object.matrix, 2);	

				if (xhxCuttingPlaneActor === undefined) {
					xhxCuttingPlaneActor = new XHX.CuttingPlaneActor();
					xhxCuttingPlaneActor.enabled = true;
					var d = lPlaneOrigin.dot(lPlaneNormal);
					xhxCuttingPlaneActor.plane = new THREE.Vector4(lPlaneNormal.x, lPlaneNormal.y, lPlaneNormal.z, d);

					//add to the scene once
					editor.execute( new AddObjectCommand( editor, xhxCuttingPlaneActor ) );
				} else {//update the plane
					var d = lPlaneOrigin.dot(lPlaneNormal);
					xhxCuttingPlaneActor.plane.set(lPlaneNormal.x, lPlaneNormal.y, lPlaneNormal.z, d);
				}

				var rm = xhxGetResultMaterial();
				var em = xhxGetEdgeMaterial(0);
			
				if (!xhxCuttingPlaneActor.enabled) {
					xhxCuttingPlaneActor.children = [];
					xhxCuttingPlaneActor.material = undefined;
			
					rm.uniforms.uCutting.value = 0; //no cutting on surface any more
					em.uniforms.uCutting.value = 0;
				} else {
					if (study !== undefined && study.data.loaded) {
						//xhxCuttingPlaneActor.convertDistance(study.data.bbox);
					}
				
					if (rm !== undefined) {
						rm.uniforms.uCutting.value = xhxCuttingPlaneActor.enabled? 1:0;
						rm.uniforms.uCuttingPlane.value = xhxCuttingPlaneActor.plane;
					}
				
					em.uniforms.uCutting.value = xhxCuttingPlaneActor.enabled? 1:0;
					em.uniforms.uCuttingPlane.value = xhxCuttingPlaneActor.plane;
				

					//if glyphs are displayed, no cuts needed
					if (!xhxGlyphActor || !xhxGlyphActor.visible)
						editor.loader.xhxLoadCuttingPlane(study, xhxCuttingPlaneActor.plane);				
					else {
						//cutting plane is enabled and changed, glyphs visibility need to be updated
						updateGlyphDisplay();
					}
				}
			}

		}

		render();

	} );
	transformControls.addEventListener( 'mouseDown', function () {

		var object = transformControls.object;

		objectPositionOnDown = object.position.clone();
		objectRotationOnDown = object.rotation.clone();
		objectScaleOnDown = object.scale.clone();

		controls.enabled = false;

	} );
	transformControls.addEventListener( 'mouseUp', function () {

		var object = transformControls.object;

		if ( object !== undefined ) {

			switch ( transformControls.getMode() ) {

				case 'translate':

					if ( ! objectPositionOnDown.equals( object.position ) ) {

						editor.execute( new SetPositionCommand( editor, object, object.position, objectPositionOnDown ) );

					}

					break;

				case 'rotate':

					if ( ! objectRotationOnDown.equals( object.rotation ) ) {

						editor.execute( new SetRotationCommand( editor, object, object.rotation, objectRotationOnDown ) );

					}

					break;

				case 'scale':

					if ( ! objectScaleOnDown.equals( object.scale ) ) {

						editor.execute( new SetScaleCommand( editor, object, object.scale, objectScaleOnDown ) );

					}

					break;

			}

		}

		controls.enabled = true;

	} );

	sceneHelpers.add( transformControls );

	// object picking

	var raycaster = new THREE.Raycaster();
	var mouse = new THREE.Vector2();

	// events

	function getIntersects( point, objects ) {

		mouse.set( ( point.x * 2 ) - 1, - ( point.y * 2 ) + 1 );

		raycaster.setFromCamera( mouse, camera );

		return raycaster.intersectObjects( objects );

	}

	var onDownPosition = new THREE.Vector2();
	var onUpPosition = new THREE.Vector2();
	var onDoubleClickPosition = new THREE.Vector2();

	function getMousePosition( dom, x, y ) {

		var rect = dom.getBoundingClientRect();
		return [ ( x - rect.left ) / rect.width, ( y - rect.top ) / rect.height ];

	}

	function handleClick() {

		if ( onDownPosition.distanceTo( onUpPosition ) === 0 ) {

			var intersects = getIntersects( onUpPosition, objects );

			if ( intersects.length > 0 ) {

				var object = intersects[ 0 ].object;

				if ( object.userData.object !== undefined ) {

					// helper

					editor.select( object.userData.object );

				} else {

					editor.select( object );

				}

			} else {

				editor.select( null );

			}

			render();

		}

	}
	
	function xhxGetResultMaterial() {
		if (this.resultMaterial === undefined) {
			var iCuttingPlaneEnabled = xhxCuttingPlaneActor && srcCuttingPlaneActor.enabled? 1:0;
			var cuttingPlane = xhxCuttingPlaneActor? srcCuttingPlaneActor.plane : new THREE.Vector4(0, 0, 0, 0);
			var uniforms = {
				"uHasResult" : {type: 'i', value: 0 },
				"uCutting" : {type: 'i', value: iCuttingPlaneEnabled },
				"uCuttingPlane" : {type: 'v4', value: cuttingPlane },
				"uSingleValueClipping" : {type: 'i', value: xhxIsoSurfaceActor && xhxIsoSurfaceActor.clipValue !== undefined? 1:0 },
				"uClipValue" : {type: 'f', value: xhxIsoSurfaceActor && xhxIsoSurfaceActor.clipValue !== undefined? xhxIsoSurfaceActor.clipValue : 0 },
				texture1: { type: 't', value: xhxPartSurfaceActor.texture}
			};
			this.resultMaterial = new THREE.ShaderMaterial({
				uniforms: uniforms,
				vertexShader: xhxRainbowShader.vertexShader,
				fragmentShader: xhxRainbowShader.fragmentShader
			});
			this.resultMaterial.side = THREE.DoubleSide;//not default?

			this.resultMaterial.polygonOffset = true
			this.resultMaterial.polygonOffsetFactor = 1;
			this.resultMaterial.polygonOffsetUnits = 1;
		}

		return this.resultMaterial;
	}

	function xhxGetEdgeMaterial(bHasResult, texture) {
		if (this.edgeMaterial === undefined) {				
			var iCuttingPlaneEnabled = xhxCuttingPlaneActor && srcCuttingPlaneActor.enabled? 1:0;
			var cuttingPlane = xhxCuttingPlaneActor? srcCuttingPlaneActor.plane : new THREE.Vector4(0, 0, 0, 0);
			var edgeMaterialUniforms = {
				"uHasResult" : {type: 'i', value: bHasResult ? 1:0 },
				texture1: { type: 't', value: texture }, //required if bHasResult is true
				"uCutting" : {type: 'i', value: iCuttingPlaneEnabled },
				"uCuttingPlane" : {type: 'v4', value: cuttingPlane },
				opacity:   { value: 1.0 },
				color:     { value: new THREE.Color( 0x000000 ) }
			};
			this.edgeMaterial = new THREE.ShaderMaterial( {
				uniforms:       edgeMaterialUniforms,
				vertexShader:   xhxRainbowShader.vertexShader_Edges,
				fragmentShader: xhxRainbowShader.fragmentShader_Edges,
				depthTest:      true,
				transparent:    false
			});            
		}

		return this.edgeMaterial;
	}

	function xhxGetTransparentMaterial() {
		if (this.transparentMaterial === undefined) {
			this.transparentMaterial = new THREE.MeshBasicMaterial( {
						color: '#00d3e1',
						opacity: 0.2,
						transparent: true
					} );
		}

		return this.transparentMaterial;
	}

	function xhxCutMaterial(normal, bHasResult, texture) {
		//each iso has to use a different material because its clip value (uniform variable) is different
		var uniforms = {
			"uPlaneNormal" : { type: "v3", value: new THREE.Vector3( normal[0], normal[1], normal[2] ) },
			"uHasResult" : {type: 'i', value: bHasResult ? 1:0 },
			texture1: { type: 't', value: texture }
		};
		var m;
		if (xhxCuttingPlaneActor.material === undefined) 
		{
			xhxCuttingPlaneActor.material = new THREE.ShaderMaterial({
					uniforms: uniforms,
					vertexShader: xhxRainbowShader.vertexShader_Cut,
					fragmentShader: xhxRainbowShader.fragmentShader_Cut
				});
			xhxCuttingPlaneActor.material.side = THREE.DoubleSide;//not default?

			m = xhxCuttingPlaneActor.material;
		} else {
			m = xhxCuttingPlaneActor.material;//.clone();
			m.uniforms.uHasResult = uniforms.uHasResult;
		}

		return m;
	}


	function xhxUpdateResultMaterialTexture(newTexture) {
        if (newTexture === undefined)
            return;

        newTexture.magFilter = THREE.NearestFilter;

        if (xhxPartSurfaceActor)
            xhxPartSurfaceActor.setTexture(newTexture);

        if (xhxCuttingPlaneActor !== undefined) {
            xhxCuttingPlaneActor.setTexture(newTexture);
        }
        if (xhxIsoSurfaceActor !== undefined) {
            xhxIsoSurfaceActor.setTexture(newTexture);
        }
    }

    function xhxDisplayPart(part, aScene) {

		var study = editor.xhxDoc.study;
		//find the index
		var idx = -1;
		for (var i = 0; i < study.data.parts.length; ++i) {
			if (study.data.parts[i].name == part.name) {
				idx = i;
				break;
			}
		}

		if (idx == -1)
			return;
		
		var partSurfaceActor = xhxPartSurfaceActor.getPart(idx);
		if (!partSurfaceActor)
			return;

      	//part may contain one or more fragments
      	for (var iFrag in part.fragments) {
          	var lFrag = part.fragments[iFrag];

			var geoFrag = lFrag.triGeo;
        
			var resultFrag = new THREE.Mesh(geoFrag, xhxGetResultMaterial());
			resultFrag.visible = true;
			partSurfaceActor.surfaceRltGroup.add(resultFrag);
	  /* no trasparent for CFD ?
			var transparentFrag = new THREE.Mesh(geoFrag, xhxGetTransparentMaterial());
			transparentFrag.visible = true; //visible only when fWarpScaleFactor > 0.0
			partSurfaceActor.transparentGroup.add(transparentFrag);
	  */
			if (lFrag.feGeo !== undefined) {
				var edgeRltFrag = new THREE.LineSegments(lFrag.feGeo, xhxGetEdgeMaterial(0));
	  
				partSurfaceActor.featureEdgeGroup.add(edgeRltFrag);
			}
		}
    }

	function xhxShowContours(aShow) {

	    xhxPartSurfaceActor.children.forEach( function(child) {
			child.surfaceRltGroup.visible = !aShow;
    	});

		if (!aShow) {
			if (xhxIsoSurfaceActor !== undefined) {
				//scene.remove(IsoSurfaceActor);
				xhxIsoSurfaceActor.visible = false;
			}
			render();
		} else {
			if (xhxIsoSurfaceActor === undefined) {
				xhxIsoSurfaceActor = new XHX.IsoSurfaceActor();
				//add to the scene once
				editor.execute( new AddObjectCommand( editor, xhxIsoSurfaceActor ) );
			} else {
				xhxIsoSurfaceActor.visible = true;
			}
			xhxIsoSurfaceActor.clipValue = undefined;//mark multiple contours are applied
			editor.loader.xhxLoadContours(xhxDoc.study, false, xhxIsoSurfaceActor.nContourCount);				
		}
	}

	function onGlyphScaleFactorChanged(aNewScaleFactor) {
		if (!xhxGlyphActor)
			return;//no warp has been applied

		xhxGlyphActor.scaleFactor = aNewScaleFactor;

		var mesh = xhxGlyphActor.children[0];
		if (mesh) {
			var mat = mesh.material;
			if (mat) {
				mat.uniforms.uGlobalScaleFactor.value = xhxGlyphActor.scaleFactor;
				render();
			}
		}
	}
	
	//call this after anything changes in the active study
	function xhxUpdateStudyDisplay(study) {
		if (study === undefined || study.data == undefined || study.data.parts == undefined)
			return;
			
		var rm = xhxGetResultMaterial(); //assume the material has been created, no need to pass texture
	 
		var bResultVisible = rm !== undefined && rm.uniforms.uHasResult.value == 1;
	 
		var rltRange = [0.0, 0.0];
		if (study.data.result !== undefined && bResultVisible) {
			rltRange = study.data.result.f2ScaleRange;
			if (rltRange !== undefined) {
				var rltInfo = study.data.results[study.data.resultIdx];
				var rltName = rltInfo === undefined? 'result name' : rltInfo.name;
				//set scale range on the legend bar
				if (xhxColorBar) {
					xhxColorBar.setScalarRange(rltRange[0], rltRange[1]);
					xhxColorBar.setTitle(rltInfo.unit);					
				}	 
				if (rm !== undefined)
					rm.uniforms.uHasResult.value = 1;
			}
		}
	 	 
		var tm = xhxGetTransparentMaterial();
	 
		for (iPart = 0; iPart < study.data.parts.length; iPart++)
		{
			var part = study.data.parts[iPart];
			if (part.fragments == undefined)
				continue;

			if (xhxPartSurfaceActor && part.elementStatusApplied !== undefined && part.elementStatusApplied == 1) {
				//the surface may have been rebuilt, rebuild the display
				if (iPart == 0) {
					xhxPartSurfaceActor.getPart(iPart).surfaceRltGroup.children = [];
				}
				for (iFrag = 0; iFrag < part.fragments.length; iFrag++) {
					var geoFrag = part.fragments[iFrag].triGeo;
					
					var resultFrag = new THREE.Mesh(geoFrag, rm);
					resultFrag.visible = true;
					xhxPartSurfaceActor.getPart(iPart).surfaceRltGroup.add(resultFrag);
				}
			} else if (study.data.result && study.data.result.parts && study.data.result.parts.length > 0) {
				if (part.elementStatusApplied == 2) {
					streamlineDisplay(true, study.data.result.parts[0].lineGeo);
					break;//only the first part contains streamline
				} else
					streamlineDisplay(false, study.data.result.parts[0].lineGeo);
			}
		}

		//transparent parts
		var bTransparentVisible = false;//xhxWarpCtrl.bUndeformedVisible;
		if (xhxPartSurfaceActor.transparentGroup != undefined)
			xhxPartSurfaceActor.transparentGroup.visible = bTransparentVisible;
	 
		//showLegend(bResultVisible);
	}

	function xhxDisplayIsoPart(part) {
		var study = xhxDoc.study;
		if (!study)
			return;

        if (study === undefined || !study.data.loaded || study.data.result === undefined)
            return;

        if (part === undefined || part.type === undefined || part.type != 'iso')
            return;//for iso only

        if (xhxIsoSurfaceActor === undefined)
            return;

        var f2 = study.data.result.f2ScaleRange;
        if (f2 === undefined || f2.length != 2)
            return;
        
        //convert to texture coordinates [0, 1]
        var cv = (part.clipValue - f2[0])/(f2[1] - f2[0]);

        var texture = xhxResultTexture;
        
        //part may contain one or more fragments
        for (var iFrag in part.fragments) {
            var lFrag = part.fragments[iFrag];

            var geoFrag = lFrag.triGeo;
            geoFrag.clipValue = cv;
            
            var m = xhxIsoSurfaceActor.isoMaterial(cv, texture);

            var isoFrag = new THREE.Mesh(geoFrag, m);
            isoFrag.onBeforeRender = this.onBeforeIsoRender;
            xhxIsoSurfaceActor.add(isoFrag);
        }
    }

    function xhxDisplayCutPart(part) {
		var study = xhxDoc.study;
		if (!study)
			return;
			  
        if (part === undefined || part.type === undefined || part.type != 'cut')
            return;//for cuts only

        var bHasResult = study.data.result !== undefined ? 1:0;
//        var rltMat = xhxGetResultMaterial();
//        if (rltMat !== undefined) //result may exist but not be used
//            bHasResult = rltMat.uniforms.uHasResult.value;
        
        var normal = xhxCuttingPlaneActor.plane;//the first 3 numbers form the normal
        
        var texture = bHasResult? xhxResultTexture : undefined;
       
        //part may contain one or more fragments
        for (var iFrag in part.fragments) {
            var lFrag = part.fragments[iFrag];

            var geoFrag = lFrag.triGeo;
            geoFrag.planeNormal = normal;
                        
            var m = xhxCutMaterial(normal, bHasResult, texture);

			var cutFrag = new THREE.Mesh(geoFrag, m);
			
            cutFrag.onBeforeRender = this.onBeforeCutRender;
            xhxCuttingPlaneActor.add(cutFrag);
        }
    }

    function xhxShowLegend(bShow) {
		if (xhxColorBar)
			xhxColorBar.setVisible(bShow? 1:0);

		 if (bShow) {
			 if (!xhxResultTexture) {
				 if (xhxColorBar) {
					xhxResultTexture = xhxColorBar.createTexture();	
				 } else {
					var colorMap = 'rainbow';
					var numberOfColors = 256;
					var rainbowLUT = new THREE.LutInOld3JS( colorMap, numberOfColors );
					var sLayout = 'vertical';
					var pos = { 'x': 0.0, 'y': 0.0, 'z': 0 };
					rainbowLUT.setLegendOn( {'layout':sLayout,  'position': pos} );//this will create the legend and create the texture	
					xhxResultTexture = rainbowLUT.legend.texture;	
				 }
			 }

			xhxUpdateResultMaterialTexture(xhxResultTexture);
		}
        else
            xhxUpdateResultMaterialTexture(undefined);
    }

	function xhxShowMeshEdges(aVisible, aPortion) {
		if (!xhxPartSurfaceActor)
			return;
		
		if (aPortion == 1) {
			if (aVisible) {
				if (xhxPartSurfaceActor.children[0].meshEdgeGroup.children.length == 0) {
					editor.loader.xhxLoadMeshEdges(xhxDoc.study, 1);				
				}
			}
			xhxPartSurfaceActor.children.forEach( function(child) {
				if (child.meshEdgeGroup !== undefined)
					child.meshEdgeGroup.traverse( function ( object ) { object.visible = aVisible; } );
			});
		} else
		if (aPortion == 2) {
			if (aVisible) {
				if (xhxPartSurfaceActor.children[0].featureEdgeGroup.children.length == 0) {
					editor.loader.xhxLoadMeshEdges(xhxDoc.study, 2);				
				}
			}
			xhxPartSurfaceActor.children.forEach( function(child) {
				if (child.featureEdgeGroup !== undefined)
					child.featureEdgeGroup.traverse( function ( object ) { object.visible = aVisible; } );
			});
		}
	}
	

	function onMouseDown( event ) {

		// event.preventDefault();

		var array = getMousePosition( container.dom, event.clientX, event.clientY );
		onDownPosition.fromArray( array );

		document.addEventListener( 'mouseup', onMouseUp, false );
	}

	function onMouseUp( event ) {

		var array = getMousePosition( container.dom, event.clientX, event.clientY );
		onUpPosition.fromArray( array );

		handleClick();

		document.removeEventListener( 'mouseup', onMouseUp, false );

	}

	function onTouchStart( event ) {

		var touch = event.changedTouches[ 0 ];

		var array = getMousePosition( container.dom, touch.clientX, touch.clientY );
		onDownPosition.fromArray( array );

		document.addEventListener( 'touchend', onTouchEnd, false );

	}

	function onTouchEnd( event ) {

		var touch = event.changedTouches[ 0 ];

		var array = getMousePosition( container.dom, touch.clientX, touch.clientY );
		onUpPosition.fromArray( array );

		handleClick();

		document.removeEventListener( 'touchend', onTouchEnd, false );

	}

	function onDoubleClick( event ) {

		var array = getMousePosition( container.dom, event.clientX, event.clientY );
		onDoubleClickPosition.fromArray( array );

		var intersects = getIntersects( onDoubleClickPosition, objects );

		if ( intersects.length > 0 ) {

			var intersect = intersects[ 0 ];

			signals.objectFocused.dispatch( intersect.object );

		}

	}

	container.dom.addEventListener( 'mousedown', onMouseDown, false );
	container.dom.addEventListener( 'touchstart', onTouchStart, false );
	container.dom.addEventListener( 'dblclick', onDoubleClick, false );

	// controls need to be added *after* main logic,
	// otherwise controls.enabled doesn't work.

	var controls = new THREE.EditorControls( camera, container.dom );
	controls.addEventListener( 'change', function () {

		signals.cameraChanged.dispatch( camera );

	} );

	// signals

	signals.editorCleared.add( function () {

		controls.center.set( 0, 0, 0 );
		render();

	} );

	signals.transformModeChanged.add( function ( mode ) {

		transformControls.setMode( mode );

	} );

	signals.snapChanged.add( function ( dist ) {

		transformControls.setTranslationSnap( dist );

	} );

	signals.spaceChanged.add( function ( space ) {

		transformControls.setSpace( space );

	} );

	signals.rendererChanged.add( function ( newRenderer ) {

		if ( renderer !== null ) {

			container.dom.removeChild( renderer.domElement );

		}

		renderer = newRenderer;

		renderer.autoClear = false;
		renderer.autoUpdateScene = false;
		renderer.gammaOutput = true;
		renderer.setPixelRatio( window.devicePixelRatio );
		renderer.setSize( container.dom.offsetWidth, container.dom.offsetHeight );

		xhxGnomeRenderer.autoClear = false;
		xhxGnomeRenderer.autoUpdateScene = false;
		xhxGnomeRenderer.gammaOutput = true;
		xhxGnomeRenderer.setPixelRatio( window.devicePixelRatio );
		xhxGnomeRenderer.setSize( container.dom.offsetWidth, container.dom.offsetHeight );

		container.dom.appendChild( renderer.domElement );

		xhxGnomeActor.init(camera.aspect, renderer.domElement);
		//synchronize the orientation of the two cameras
		controls.updateCamera(xhxGnomeActor.cameraGnome)

		render();

	} );

	signals.sceneGraphChanged.add( function () {

		render();

	} );

	signals.cameraChanged.add( function () {

		if (editor.xhxLight !== undefined) {
			//xhxLight is added in Loader.js/xhxLoad
			//is there a better way to keep the light at camera position?
			editor.xhxLight.position.copy(camera.position);
		}

		if (controls && xhxGnomeActor)
			controls.updateCamera(xhxGnomeActor.cameraGnome)

		render();

	} );

	signals.objectSelected.add( function ( object ) {

		selectionBox.visible = false;
		transformControls.detach();

		if ( object !== null && object !== scene && object !== camera ) {

			box.setFromObject( object );

			if ( box.isEmpty() === false ) {

				selectionBox.setFromObject( object );
				selectionBox.visible = true;

			}

			transformControls.attach( object );
		}

		render();

	} );

	signals.objectFocused.add( function ( object ) {

		controls.focus( object );

	} );

	signals.geometryChanged.add( function ( object ) {

		if ( object !== undefined ) {

			selectionBox.setFromObject( object );

		}

		render();

	} );

	signals.objectAdded.add( function ( object ) {

		object.traverse( function ( child ) {

			objects.push( child );

		} );

	} );

	signals.objectChanged.add( function ( object ) {

		if ( editor.selected === object ) {

			selectionBox.setFromObject( object );

		}

		if ( object.isPerspectiveCamera ) {

			object.updateProjectionMatrix();

		}

		if ( editor.helpers[ object.id ] !== undefined ) {

			editor.helpers[ object.id ].update();

		}

		render();

	} );

	signals.objectRemoved.add( function ( object ) {

		controls.enabled = true; // see #14180
		if ( object === transformControls.object ) {

			transformControls.detach();

		}

		object.traverse( function ( child ) {

			objects.splice( objects.indexOf( child ), 1 );

		} );

	} );

	signals.helperAdded.add( function ( object ) {

		objects.push( object.getObjectByName( 'picker' ) );

	} );

	signals.helperRemoved.add( function ( object ) {

		objects.splice( objects.indexOf( object.getObjectByName( 'picker' ) ), 1 );

	} );

	signals.materialChanged.add( function ( material ) {

		render();

	} );

	// fog

	signals.sceneBackgroundChanged.add( function ( backgroundColor ) {

		scene.background.setHex( backgroundColor );

		render();

	} );

	var currentFogType = null;

	signals.sceneFogChanged.add( function ( fogType, fogColor, fogNear, fogFar, fogDensity ) {

		if ( currentFogType !== fogType ) {

			switch ( fogType ) {

				case 'None':
					scene.fog = null;
					break;
				case 'Fog':
					scene.fog = new THREE.Fog();
					break;
				case 'FogExp2':
					scene.fog = new THREE.FogExp2();
					break;

			}

			currentFogType = fogType;

		}

		if ( scene.fog ) {

			if ( scene.fog.isFog ) {

				scene.fog.color.setHex( fogColor );
				scene.fog.near = fogNear;
				scene.fog.far = fogFar;

			} else if ( scene.fog.isFogExp2 ) {

				scene.fog.color.setHex( fogColor );
				scene.fog.density = fogDensity;

			}

		}

		render();

	} );

	signals.viewportCameraChanged.add( function ( viewportCamera ) {

		if ( viewportCamera.isPerspectiveCamera ) {

			viewportCamera.aspect = editor.camera.aspect;
			viewportCamera.projectionMatrix.copy( editor.camera.projectionMatrix );

		} else if ( ! viewportCamera.isOrthographicCamera ) {

			throw "Invalid camera set as viewport";

		}

		camera = viewportCamera;

		render();

	} );

	//

	signals.windowResize.add( function () {

		var width = container.dom.offsetWidth;
		var height = container.dom.offsetHeight;

		editor.DEFAULT_CAMERA.aspect = width / height;
		editor.DEFAULT_CAMERA.updateProjectionMatrix();

		camera.aspect = editor.DEFAULT_CAMERA.aspect;
		camera.updateProjectionMatrix();

		//for the floating buttons on the right
		if (!IsInMode("openfoam")) {
			btnCamera.aspect = camera.aspect;
			btnCamera.updateProjectionMatrix();
			adjustBtnPosition(btns, btnCamera);	
		}

		renderer.setSize( width, height );

		//resize the 2d canvas
		var textCanvas = document.getElementById("text");
		if (textCanvas) {
			var ctx = textCanvas.getContext("2d");
			if (ctx) {
				currentPixelRatio = renderer.getPixelRatio();
				ctx.canvas.width = Math.floor( width * currentPixelRatio );
				ctx.canvas.height = Math.floor( height * currentPixelRatio );
			}
		}

		//the color bar
		if (xhxColorBar) {
			xhxColorBar.setPosition(Math.floor(0.5 * (width - xhxColorBar.getWidth())), 50);
		}

		xhxGnomeActor.onWindowResize(camera.aspect);

		render();

	} );

	signals.showGridChanged.add( function ( showGrid ) {

		grid.visible = showGrid;
		render();

	} );

	function enableTranslateCtrl(aProps) {
		if (aProps.enable) {
			var object = editor.selected;
			if (!object && xhxCuttingPlaneMesh) {
				object = xhxCuttingPlaneMesh;
				editor.execute(new AddObjectCommand(editor, object));
			}
			if (!object) {
				//created a plane, a transform control will be activated after the plane
				//is added in the editor. See line 450 of editor.js
				var geometry = new THREE.PlaneBufferGeometry( 1, 1, 1, 1 );
				var material = new THREE.MeshBasicMaterial( {
				color: '#00d3e1',
				opacity: 0.1,
				transparent: true,
				side: THREE.DoubleSide,
				} );
				var mesh = new THREE.Mesh( geometry, material );
				mesh.name = 'Plane';

				var study = editor.xhxDoc.study;
				if (study) {
					var lSize = study.data.size;
					//change the plane size
					geometry.scale(1.2 * lSize, 1.2 * lSize, 1, 1);

					var bb = study.data.bbox;
					var center = bb.getCenter();
					mesh.position.add(center);
				}

				editor.execute( new AddObjectCommand( editor, mesh ) );
			}
			
			editor.signals.transformModeChanged.dispatch(aProps.method);
        } else {
			var object = editor.selected;
			if (object) {
			  	var parent = object.parent;
			  	if ( parent !== undefined )
			  	{
				  	xhxCuttingPlaneMesh = object;
					editor.select(null);//unselect to remove the yellow box
//					editor.execute( new RemoveObjectCommand( object ) ); //remove the plane and the transform ctrl.
					object.visible = false;
			  	}	  
			}
		  }
	}

	//nCmpts is used to determine glyph type: 1: sphere, 3: dart
	function displayGlyphs(aShow, aPositions, aColorValues, aCmpts, aOrientationVector) {
		if (!aShow) {
			xhxPartSurfaceActor.children.forEach( function(child) {
				child.surfaceRltGroup.visible = true;	
			});

			if (xhxGlyphActor !== undefined) {
				//scene.remove(IsoSurfaceActor);
				xhxGlyphActor.visible = false;
				editor.execute( new RemoveObjectCommand( xhxGlyphActor ) );
				xhxGlyphActor = undefined;
			}

			if (xhxCuttingPlaneActor) {
				xhxCuttingPlaneActor.visible = true;
			}

			render();

			return;
		}

		if (aPositions == undefined)
			return;

		var study = xhxDoc.study;

		//determine glyph size according to study bb
		var scaleFactor = study.data.size * 0.0075;

		//hide surface
		if (xhxPartSurfaceActor != undefined) {
			xhxPartSurfaceActor.children.forEach( function(child) {
				child.surfaceRltGroup.visible = false;	
			});
		}	

		//hide cut if exists
		var bHasCuttingPlane = false;
		var planeABCD;
		var lCutHalfWidth = 1.0;
		if (xhxCuttingPlaneActor) {
			xhxCuttingPlaneActor.visible = false;
			bHasCuttingPlane = true;
			planeABCD = xhxCuttingPlaneActor.plane;//a Vector4

			lCutHalfWidth = xhxCuttingPlaneActor.cutHalfWidthScaleFactor * (study.data.size * 0.0075); //for glyph on cutting plane
		}

		if (xhxGlyphActor === undefined) {
			xhxGlyphActor = new XHX.GlyphActor();
			//add to the scene once
			editor.execute( new AddObjectCommand( editor, xhxGlyphActor ) );
		} else {
			xhxGlyphActor.visible = true;
		}
		xhxGlyphActor.children = [];//reset to create new mesh

		// geometry
		var positions = aPositions;
		var instances = positions.length/3;

		const matArraySize = instances * 4;
		const matrixArray = [
			  new Float32Array(matArraySize),
			  new Float32Array(matArraySize),
			  new Float32Array(matArraySize),
			  new Float32Array(matArraySize),
		];

		const glyphVisibility = new Float32Array(instances);

		var bufferGeometry;
		if (aCmpts == 3)
			bufferGeometry = new THREE.ConeBufferGeometry( 1.0, 4.0, 32);
		else
			bufferGeometry = new THREE.SphereBufferGeometry( 1.0, 32, 32);

		// copying data from a simple box geometry, but you can specify a custom geometry if you want

		var geometry = new THREE.InstancedBufferGeometry();
		geometry.index = bufferGeometry.index;
		geometry.attributes.position = bufferGeometry.attributes.position;

		// per instance data
		var orientations = [];
		var ts = [];
		if (aColorValues != undefined) 
			ts = aColorValues;//assume it has been normalized

		var vector = new THREE.Vector4();
		var x, y, z, w;

		var start = new THREE.Vector3(0, 1, 0);
		var dest = new THREE.Vector3(0, -1, 0);
		var q = new THREE.Quaternion();

		var object = new THREE.Object3D() //we'll swap out the mesh with this
		for ( var i = 0; i < instances; i ++ ) {
		//position
			dest.x = positions[i * 3];
			dest.y = positions[i * 3 + 1];
			dest.z = positions[i * 3 + 2];
			
			glyphVisibility[i] = 1;
			if (bHasCuttingPlane) {
				var distance = dest.x * planeABCD.x + dest.y * planeABCD.y + dest.z * planeABCD.z - planeABCD.w;
				if (xhxCuttingPlaneActor.glyphOnCutBothSides) {
					if (distance > lCutHalfWidth || distance < -lCutHalfWidth)
						glyphVisibility[i] = 0; //not visible
					else
						glyphVisibility[1] = 1;
				} else {
					if (distance > lCutHalfWidth)
						glyphVisibility[i] = 0;
					else
						glyphVisibility[i] = 1;
				}
			}

			object.position.copy(dest);
		//orientation
			if (aOrientationVector != undefined) {
				dest.x = aOrientationVector[i * 3];
				dest.y = aOrientationVector[i * 3 + 1];
				dest.z = aOrientationVector[i * 3 + 2];	
			}
			var len = dest.normalize(); //start is already a unit vector
			q.setFromUnitVectors(start, dest);

			object.quaternion.copy(q);
		//scale
			object.scale.x = scaleFactor;
			object.scale.y = scaleFactor;
			object.scale.z = scaleFactor;
		//
			object.updateMatrixWorld() //we compute the matrix based on position, rotation and scale

			for ( let r = 0 ; r < 4 ; r ++ ) {
			  	for ( let c = 0 ; c < 4 ; c ++ ) {
					matrixArray[r][i*4 + c] = object.matrixWorld.elements[r*4 + c]
				}
			}

			if (aColorValues == undefined)
				ts.push((1.0 * i)/(instances - 1));
		}

		m0Attribute = new THREE.InstancedBufferAttribute( new Float32Array( matrixArray[0] ), 4 );
		m1Attribute = new THREE.InstancedBufferAttribute( new Float32Array( matrixArray[1] ), 4 );
		m2Attribute = new THREE.InstancedBufferAttribute( new Float32Array( matrixArray[2] ), 4 );
		m3Attribute = new THREE.InstancedBufferAttribute( new Float32Array( matrixArray[3] ), 4 );

		var tAttribute = new THREE.InstancedBufferAttribute( new Float32Array( ts ), 1 );
		var gVisibilityAttribute = new THREE.InstancedBufferAttribute( new Float32Array( glyphVisibility ), 1 );
		orientationAttribute = new THREE.InstancedBufferAttribute( new Float32Array( orientations ), 4 ).setDynamic( true );

		geometry.addAttribute( 'aInstanceMatrix0', m0Attribute );
		geometry.addAttribute( 'aInstanceMatrix1', m1Attribute );
		geometry.addAttribute( 'aInstanceMatrix2', m2Attribute );
		geometry.addAttribute( 'aInstanceMatrix3', m3Attribute );

		geometry.addAttribute( 't', tAttribute );
		geometry.addAttribute( 'visibility', gVisibilityAttribute );

		// material

		var material = new THREE.RawShaderMaterial( {

			uniforms: {
				map: { value: xhxColorBar.createTexture() },
				"uGlobalScaleFactor" : {type: 'f', value: xhxGlyphActor.scaleFactor },
			},
			vertexShader: xhxRainbowShader.vertexShader_Instanced,
			fragmentShader: xhxRainbowShader.fragmentShader_Instanced
		} );

		mesh = new THREE.Mesh( geometry, material );
		mesh.name = "glyph_instances_mesh";

		xhxGlyphActor.add(mesh);

		render();
	}

	var cbGlyphDisplay = function(rlt) {
		if (rlt == undefined)
			return;
		var positions = rlt.positions;

		var nInstances = positions.length / 3; //number of glyphs

		var colors = new Float32Array(nInstances);
		if (rlt.nCmpts > 0 && rlt.rltValues != undefined) {
			var nCmpts = rlt.nCmpts;
			var h = 0.0;
			var l = 0.0;
			for (i = 0; i < nInstances; i++) {
				colors[i] = rlt.rltValues[i * nCmpts];
				
				if (i == 0) {
					h = colors[i];
					l = colors[i];
				} else {
					if (h < colors[i])
						h = colors[i];
					if (l > colors[i])
						l = colors[i];
				}
			}
			
			if (h > l) {
				for (i = 0; i < nInstances; i++) {
					colors[i] = (colors[i] - l) / (h - l);
				}
			}	
		} else { //take x coordinates as the color
			var h = 0.0;
			var l = 0.0;
			for (i = 0; i < nInstances; i++) {
				colors[i] = positions[i * 3];
				
				if (i == 0) {
					h = colors[i];
					l = colors[i];
				} else {
					if (h < colors[i])
						h = colors[i];
					if (l > colors[i])
						l = colors[i];
				}
			}
			
			if (h > l) {
				for (i = 0; i < nInstances; i++) {
					colors[i] = (colors[i] - l) / (h - l);
				}
			}	
		}

		var orientation = undefined;
		if (rlt.nCmpts == 3)
			orientation = rlt.rltValues;

		displayGlyphs(true, positions, colors, rlt.nCmpts, orientation);
	};

	var streamlineDisplay = function(aShow, lineGeo) {
		//
//		xhxPartSurfaceActor.children.forEach( function (child) {child.surfaceRltGroup.visible = !aShow;});
		//streamline is on the first part only
		xhxPartSurfaceActor.children[0].surfaceRltGroup.visible = !aShow;

		if (!aShow) {
			if (xhxStreamlineActor !== undefined) {
				//scene.remove(IsoSurfaceActor);
				xhxStreamlineActor.visible = false;
			}
			render();
		} else {
			if (lineGeo == undefined)
				return;
			var bHasResult = lineGeo.attributes.uv != undefined;
			var texture = xhxResultTexture;
			if (bHasResult && !texture)
				texture = xhxColorBar.createTexture();
	
			this.edgeMaterial = undefined;//need to update it
			var mshFrag = new THREE.LineSegments(lineGeo, xhxGetEdgeMaterial(bHasResult, texture));
	
			if (xhxStreamlineActor === undefined) {
				xhxStreamlineActor = new XHX.StreamlineActor();
				xhxStreamlineActor.add(mshFrag);
				//add to the scene once
				editor.execute( new AddObjectCommand( editor, xhxStreamlineActor ) );
			} else {
				xhxStreamlineActor.visible = true;
				xhxStreamlineActor.children[0] = mshFrag; //replace the old with the new
			}
		}
	};

	signals.xhxProjectSettingsChanged.add( function (whatChanged, changedValue, value2 ) {
		switch (whatChanged) {
			case 0: {//contour enabled
				xhxShowContours(changedValue);
			}
			break;
			case 1: {//number of contours changed
				var n = changedValue;
				xhxIsoSurfaceActor.nContourCount = n;
				editor.loader.xhxLoadContours(xhxDoc.study, false, xhxIsoSurfaceActor.nContourCount);	
			}
			break;
			case 2: {//reset view
				var study = editor.xhxDoc.study;
				if (study && study.data.bbox && xhxPartSurfaceActor) {
					//some parts may not be visible
					var nVisibleParts = 0;
					var visibleBB;
					for (var iPart in study.data.parts) {
						var partActor = xhxPartSurfaceActor.getPart(iPart);
						if (!partActor || !partActor.visible)
							continue;

						var part = study.data.parts[iPart];
						
						if (!nVisibleParts) {
							visibleBB = new THREE.Box3();
							visibleBB.copy(part.bbox);
						} else
							visibleBB.union(part.bbox);

						nVisibleParts++;
					}

					if (nVisibleParts) {
						if (controls !== undefined)
							controls.center.copy(visibleBB.getCenter());
				
						if (camera !== undefined)
							XHX.resetCamera(camera, visibleBB, 2);
					}
				}
			}
			break;
			case 3: {//set view center
				var cX = changedValue.clientX;
				var cY = changedValue.clientY;

				var mouse = new THREE.Vector2();
				mouse.x = ( cX / renderer.domElement.clientWidth ) * 2 - 1;
				mouse.y = - ( cY / renderer.domElement.clientHeight ) * 2 + 1;

				var raycaster = new THREE.Raycaster();
				raycaster.setFromCamera( mouse, camera );

				// See if the ray from the camera into the world hits one of our meshes
				var intersects = raycaster.intersectObject(scene, true);

				if ( intersects.length > 0 ) {
					var pos;//fill out line segments, somehow, many lines may be selected.
					for ( var i = 0; i < intersects.length; i++ ) {
						if (intersects[i].face) {
							pos = intersects[i].point;
							break;
						}
					}

					if (pos) {
						grid.position.copy(pos);
						controls.recenter(pos);
					}
				}
			}
			break;
			case 4: {
				if (changedValue != undefined) {
					switch (changedValue) {
						case 0: {//enable/disable anchor plane
							enableWarpTool(1, value2);
						}
						break;
						case 1: {//1st anchor node
						}
						break;
						case 2: {//2nd anchor node
						}
						break;
						case 3: {//3rd anchor node
						}
						break;
					}
				}
			}
			break;
			case 5: {//mesh edges display
				xhxShowMeshEdges(changedValue, 1);
				render();
			}
			break;
			case 6: {//feature edges display
				xhxShowMeshEdges(changedValue, 2);
				render();
			}
			break;
			case 7: {//warp measure tool
				enableWarpTool(2, value2);
			}
			break;
			case 8: {//glyph display				
				var bChecked = changedValue;

				if (!bChecked) {
					displayGlyphs(false);  
				} else {
					var study = xhxDoc.study;	
					editor.loader.xhxLoadGlyphs(study, cbGlyphDisplay);					
				}
			}
			break;
			case 9: {//glyph scale factor
				if (xhxGlyphActor) {
					var scaleFactor = changedValue;
					onGlyphScaleFactorChanged(scaleFactor);
				}
			}
			break;
			case 10: {//show select result dialog
				showSelectRltDialog(true);
			}
			break;
			case 11: {//part surface display
				if (xhxPartSurfaceActor) {
					xhxPartSurfaceActor.children.forEach(function(child) {
						child.surfaceRltGroup.traverse( function ( object ) { object.visible = changedValue; } );
					});
					render();
				}
			}
			break;
			case 12: {//12: flip cutting plane normal
				if (transformControls && xhxCuttingPlaneActor) {
					var obj = transformControls.object;
					if (obj) {
						var opt = changedValue;

						var lPlaneOrigin = obj.position;
						var lPlaneNormal = new THREE.Vector3();
						lPlaneNormal.setFromMatrixColumn(obj.matrix, 2);	
						//reverse the vector
						lPlaneNormal.multiplyScalar(-1);

						//update cut
						var d = lPlaneOrigin.dot(lPlaneNormal);
						xhxCuttingPlaneActor.plane.set(lPlaneNormal.x, lPlaneNormal.y, lPlaneNormal.z, d);

						//update the object (the displayed plane)
						if (opt.axis == 'X')
							obj.rotateX(opt.angle);
						else if (opt.axis == 'Y')
							obj.rotateY(opt.angle);
						else if (opt.axis == 'Z')
							obj.rotateZ(opt.angle);
						
						//update the transform control
						transformControls.detach(obj);
						transformControls.attach(obj);

						render();
					}
				}
			}
			break;
			case 13: //enable/disable translate control
			{
				enableTranslateCtrl(changedValue);
			}	
			break;
			case 14: //glyphs on cutting plane
			{
				if (xhxCuttingPlaneActor) {
					var opt = changedValue;

					xhxCuttingPlaneActor.glyphOnCutBothSides = !opt.onOneSide;
					xhxCuttingPlaneActor.cutHalfWidthScaleFactor = opt.halfWidthScaleFactor;

					updateGlyphDisplay();
				}
			}	
			break;
			case 15: //set part visibility
			{
				if (xhxPartSurfaceActor) {
					var visible = changedValue.visible;
					var partIdx = changedValue.partIdx;
					var partActor = xhxPartSurfaceActor.getPart(partIdx);
					if (partActor) {
						partActor.visible = visible;
					}
				}
			}	
			break;
			case 16: //Create/Delete cutting plane
			{
				var cmdName = changedValue.cmdName;
				if (cmdName == 'New') {
					//create a plane and enable the control with rotate as the choice
					enableTranslateCtrl({enable:true, method:'rotate'});
				} else if (cmdName == 'Delete') {
					//disable the control, if it is activated
					enableTranslateCtrl({enable:false});
					//delete the cutting plane actor
					if (xhxCuttingPlaneActor) {
						//remove the actor from the scene
//						editor.execute( new RemoveObjectCommand( editor, xhxCuttingPlaneActor ) );
						editor.scene.remove(xhxCuttingPlaneActor);
						//destroy it
						xhxCuttingPlaneActor = undefined;
						xhxCuttingPlaneMesh = undefined;
						//update the display (update the material)
						if (this.resultMaterial)
							this.resultMaterial.uniforms.uCutting.value = 0;
						if (this.edgeMaterial)
							this.edgeMaterial.uniforms.uCutting.value = 0;
					}
				}
			}
			break;
			case 17: {//grid line display
				var visible = changedValue.visible;
				grid.visible = visible;
			}
			break;
		}
	} );

	signals.xhxColorBarSettingsChanged.add( function (whatChanged, changedValue ) {

		if (!xhxColorBar) 
		   return;

		if (whatChanged != 0 && whatChanged != 1)
			return;

		if (whatChanged == 0)
		{
			xhxColorBar.setColorMapName(changedValue);
		} else if (whatChanged == 1) {
			xhxColorBar.setNumberOfColors(changedValue);
		}

		xhxResultTexture = xhxColorBar.createTexture();	
		xhxUpdateResultMaterialTexture(xhxResultTexture);

		//update the shader (material). Geometry attributes have been done in the loader when the updated information is loaded.
		var rm = xhxGetResultMaterial();
		if (rm) {
			rm.uniforms.texture1.value = xhxPartSurfaceActor.texture;
		}

		render();

	} );

	signals.xhxLoaded.add( function ( loadType, obj ) {
		if (obj === undefined)
			return; //nothing is loaded
		switch (loadType) {
			case 0: {//study has been just loaded
				var study = obj;
				var bb = study.data.bbox;
				//adjust the camera according to the new bounding box
				XHX.resetCamera(editor.camera, bb, 2);
				//update the grid size
				var boxSize = bb.getSize();
				var size = boxSize.x;
				if (size < boxSize.y)
					size = boxSize.y;
				if (size < boxSize.z)
					size = boxSize.z;
				study.data.size = size;
	
				var center = bb.getCenter();
				var zxGrid = new THREE.GridHelper(2 * size, 10);
				zxGrid.position.set(center.x, center.y, center.z);
				grid.copy(zxGrid);
	
				//set the rotation center
				controls.center.copy(center);
	
				if (editor.xhxLight === undefined) {
					var light = new THREE.DirectionalLight( 0xffffff, 1.0 );
					light.name = "xhxDirectionalLight";
					light.position.copy(editor.camera.position);
					//remember this so that its position can follow the camera in Viewport.js/signials.cameraChanged
					editor.xhxLight = light; 
					editor.scene.add( light );	
				} else if (editor.scene.getObjectByName("xhxDirectionalLight") === undefined) {
					editor.scene.add(editor.xhxLight);
				}	

				XHX.infoUpdate(editor, "Mesh");//display mesh by default

				//turn on featue edges display by default
				editor.signals.xhxProjectSettingsChanged.dispatch(6, true);//6: feature edges display
			}
			break;
			case 1: {//study list has just been loaded
				var studyList = obj;
				var studyNames = [];
				for ( var i = 0; i < studyList.length; i++ ) {
					var study = studyList[i];
					if (IsInMode("openfoam"))
						studyNames.push(study.info.name);
					else
						studyNames.push(study.info.name);
				}
				showOpenCaseDialog(true, studyNames);
			}
			break;
			case 2: {//part (mesh) has been just loaded
				var part = obj;//the passed-in is a part
				xhxDisplayPart(part);
				//adjust the camera according to the new bounding box
				var bb = editor.xhxDoc.study.data.bbox;
				XHX.resetCamera(editor.camera, bb, 2);
			}
			break;
			case 3: {//result has been just loaded
				var study = obj;//the study is passed in because it holds both the result and its index
				var result = study.data.result;
				var resultIdx = study.data.resultIdx;
				if (resultIdx >= 0) {
					var rltInfo = study.data.results[resultIdx];
					var infoStr = rltInfo.name + " [" + rltInfo.unit + "]";
					XHX.infoUpdate(editor, infoStr, rltInfo.indpVals?rltInfo.indpVals[0]:undefined);
				} else {//if no result selected, mesh is displayed
					XHX.infoUpdate(editor, "Mesh");
				}

				xhxUpdateStudyDisplay(study);

				xhxShowLegend(true);//build the texture xhxPartSurfaceActor.texture
				//update the shader (material). Geometry attributes have been done in the loader when the updated information is loaded.
				var rm = xhxGetResultMaterial();
				if (rm) {
					rm.uniforms.uHasResult.value = 1;
					rm.uniforms.texture1.value = xhxPartSurfaceActor.texture;
				}

				if (xhxCuttingPlaneActor) //cutting plane needs to be reloaded
					editor.loader.xhxLoadCuttingPlane(study, xhxCuttingPlaneActor.plane);				

				if (xhxIsoSurfaceActor) {
					editor.loader.xhxLoadContours(study, false, xhxIsoSurfaceActor.nContourCount);				
				}

				var rltRange = study.data.result.f2ScaleRange;
				if (rltRange !== undefined) {
					var rltInfo = study.data.results[resultIdx];
					xhxColorBar.setScalarRange(rltRange[0], rltRange[1]);
					xhxColorBar.setTitle("");//rltInfo.unit); //under result name, no title is needed
				}

				xhxColorBar.setVisible(1);				

				//the dialog may need to be synchronized
				if (forceUpdateDisplayOptionsDlg)
					forceUpdateDisplayOptionsDlg();
			}
			break;
			case 4: {//project is loaded from server but has not been passed to study and parts may have not been loaded.
				var project = obj;
				//always create a new to overrite the old one
				editor.clear(); //reset everything
				xhxPartSurfaceActor = new XHX.PartSurfaceActor();
				//create a part actor for each part (mesh component)
				for (var i = 0; i < project.parts.length; ++i)
					xhxPartSurfaceActor.getPart(i);

				editor.execute( new AddObjectCommand( editor, xhxPartSurfaceActor ) );

				xhxColorBar.setVisible(0);
				//update the shader (material). Geometry attributes have been done in the loader when the updated information is loaded.
				var rm = xhxGetResultMaterial();
				if (rm) {
					rm.uniforms.uHasResult.value = 0;
					rm.uniforms.texture1.value = undefined;
				}
			}
			break;
			case 5: {//display mesh
				var study = obj;
				xhxShowLegend(false);
				//update the shader (material). Geometry attributes have been done in the loader when the updated information is loaded.
				if (study) {
					study.data.resultIdx = -1;
					study.data.result = undefined;
				}

				var rm = xhxGetResultMaterial();
				if (rm) {
					rm.uniforms.uHasResult.value = 0;
					rm.uniforms.texture1.value = undefined;
				}			
				
				if (xhxCuttingPlaneActor) {
					var cm = xhxCuttingPlaneActor.material;
					if (cm) {
						cm.uniforms.uHasResult.value = 0;
						cm.uniforms.texture1.value = undefined;
//						cm.uniformsNeedUpdate = true;
					}
				}

				XHX.infoUpdate(editor, "Mesh");
			}
			break;
			case 6: {//cutting plane loaded
				var parts = obj;
				if (parts) {
					if (xhxCuttingPlaneActor) {
						xhxCuttingPlaneActor.children = []; //remove old cuts to display new ones
					}
					for (var iPart = 0; iPart < parts.length; iPart++)
						xhxDisplayCutPart(parts[iPart]);
				}
			}
			break;
			case 7: {
				var parts = obj;
				if (parts) {
					if (xhxIsoSurfaceActor !== undefined) {
						//	scene.remove(IsoSurfaceActor);
						xhxIsoSurfaceActor.children = [];//clear all to remove old iso-surfaces
					}
	
					for (var iPart = 0; iPart < parts.length; iPart++)
						xhxDisplayIsoPart(parts[iPart]);
				}
			}
			break;
			case 8: {//display mesh edges
				var study = obj.study;
				var portion = obj.portion;
				if (study) {
					for (var iPart in study.data.parts) {
						var part = study.data.parts[iPart];
						for (var iFrag in part.fragments) {
							var lFrag = part.fragments[iFrag];
				
							if (portion == 1) {
								var geoFrag = lFrag.meshEdgeGeo;
								if (geoFrag) {
									var mshFrag = new THREE.LineSegments(geoFrag, xhxGetEdgeMaterial(0));
									xhxPartSurfaceActor.getPart(iPart).meshEdgeGroup.add(mshFrag);	
								}	
							} else if (portion == 2) {
								var geoFrag = lFrag.feGeo;
								if (geoFrag) {
									var mshFrag = new THREE.LineSegments(geoFrag, xhxGetEdgeMaterial(0));
									xhxPartSurfaceActor.getPart(iPart).featureEdgeGroup.add(mshFrag);	
								}	
							}
						}
					}	
				}
			}
			break;
			case 9: {//load result
				//obj contains input parameters
				var study = obj.study;
				var lSelectedRltIdx = obj.rltIdx;
				var lSelectedRltIndpIdx = obj.rltIndpIdx;
				if (xhxGlyphActor !== undefined) {
					study.data.resultIdx = lSelectedRltIdx;
					study.data.resultIndpIdx = lSelectedRltIndpIdx;
					//glyph is being displayed, load glyph
					editor.loader.xhxLoadGlyphs(study, cbGlyphDisplay);					
				} else {
					if (study && lSelectedRltIdx !== undefined) {
						if (lSelectedRltIdx == -1) {
							signals.xhxLoaded.dispatch(5, study); //5: display mesh, mesh has not been changed
						} else if (study.data.loaded = true) {
							//results can be loaded only after ALL parts have been loaded
							editor.loader.xhxLoadResult(study, lSelectedRltIdx, lSelectedRltIndpIdx);
						}
					}	
				}
			}
			break;
			case 10: {//indp idx changed
				var study = obj.study;
				var lSelectedRltIdx = study.data.resultIdx;
				var lSelectedRltIndpIdx = obj.rltIndpIdx;
				if (xhxGlyphActor != undefined && xhxGlyphActor.visible) {
					study.data.resultIdx = lSelectedRltIdx;
					study.data.resultIndpIdx = lSelectedRltIndpIdx;
					editor.loader.xhxLoadGlyphs(study, cbGlyphDisplay);					
				} else {
					editor.loader.xhxLoadResult(study, lSelectedRltIdx, lSelectedRltIndpIdx);
				}
			}
			break;
			case 11: {//delete cutting plane
				if (xhxCuttingPlaneActor) {
					xhxCuttingPlaneActor.enabled = 0;

					var rm = xhxGetResultMaterial();
					var em = xhxGetEdgeMaterial(0);
				
					{
						xhxCuttingPlaneActor.children = [];
						xhxCuttingPlaneActor.material = undefined;
				
						rm.uniforms.uCutting.value = 0; //no cutting on surface any more
						em.uniforms.uCutting.value = 0;
					}

					xhxCuttingPlaneActor = undefined;
				}
			}
			break;
		}

		render();
	} );

	// animations

	var prevTime = performance.now();

	function animate( time ) {

		if (!IsInMode("openfoam"))
			objectControls.update();

		requestAnimationFrame( animate );

	/* disable mixer, it does not include the floating buttons
		var mixer = editor.mixer;
		if ( mixer.stats.actions.inUse > 0 ) {

			mixer.update( ( time - prevTime ) / 1000 );
			render();
		}
	*/
		render();

		prevTime = time;

	}

	requestAnimationFrame( animate );

	//

	function render() {

	    renderer.clear();
    	renderer.setViewport(0, 0, container.dom.offsetWidth, container.dom.offsetHeight);
    	renderer.setScissor(0, 0, container.dom.offsetWidth, container.dom.offsetHeight);
		scene.updateMatrixWorld();
		
		renderer.render( scene, camera );
		if (!IsInMode("openfoam"))	
			renderer.render( btnScene, btnCamera);

    	renderer.clearDepth();
	

		if ( renderer instanceof THREE.RaytracingRenderer === false ) {

			if ( camera === editor.camera ) {

				sceneHelpers.updateMatrixWorld();
				renderer.render( sceneHelpers, camera );
				
				if (xhxColorBar)
					xhxColorBar.render();
			}
		}

		if (container.dom) {
			var rect = container.dom.getBoundingClientRect();
			xhxGnomeActor.render(renderer, rect);	
		}
	}

	return container;

};		
