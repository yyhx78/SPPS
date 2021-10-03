var XHX = XHX || {};

( function () {

	'use strict';

	var screenRadius = function(xyz, center, cameraDir, near) {
		var p2c = xyz.clone().sub(center); //vector from center to xyz (one of the 8 corners)
		near.value = p2c.dot(cameraDir); //projection on cameraDir
		var r2 = p2c.lengthSq() - near.value * near.value; //squared radius (r^2 + near^2 = p2c^2)
	
		return Math.sqrt(r2);
	}

	XHX.resetCamera = function(c, bb, scale) {//c: camera, bb: bounding box
		if (c === undefined || bb === undefined)
			return;
			
		var center = bb.getCenter();
	
		var scope = this;

		var cameraV = c.getWorldDirection();
		var near = {}; //object passed by reference
		var r1 = screenRadius(bb.min, center, cameraV, near);
		var maxNearDistance = near.value;
		var maxR = r1;
		r1 = screenRadius(bb.max, center, cameraV, near);
		if (near.value > maxNearDistance)
			maxNearDistance = near.value;
		if (maxR < r1) maxR = r1;
		var p = new THREE.Vector3(bb.max.x, bb.min.y, bb.min.z)
		r1 = screenRadius(p, center, cameraV, near);
		if (near.value > maxNearDistance)
			maxNearDistance = near.value;
		if (maxR < r1) maxR = r1;
		p.set(bb.max.x, bb.max.y, bb.min.z)
		r1 = screenRadius(p, center, cameraV, near);
		if (near.value > maxNearDistance)
			maxNearDistance = near.value;
		if (maxR < r1) maxR = r1;
		p.set(bb.max.x, bb.min.y, bb.max.z)
		r1 = screenRadius(p, center, cameraV, near);
		if (near.value > maxNearDistance)
			maxNearDistance = near.value;
		if (maxR < r1) maxR = r1;
		p.set(bb.min.x, bb.max.y, bb.min.z)
		r1 = screenRadius(p, center, cameraV, near);
		if (near.value > maxNearDistance)
			maxNearDistance = near.value;
		if (maxR < r1) maxR = r1;
		p.set(bb.min.x, bb.max.y, bb.max.z)
		r1 = screenRadius(p, center, cameraV, near);
		if (near.value > maxNearDistance)
			maxNearDistance = near.value;
		if (maxR < r1) maxR = r1;
		p.set(bb.min.x, bb.min.y, bb.max.z)
		r1 = screenRadius(p, center, cameraV, near);
		if (near.value > maxNearDistance)
			maxNearDistance = near.value;
		if (maxR < r1) maxR = r1;
	
		var newPos = center.add(cameraV.multiplyScalar(-scale * (maxR + maxNearDistance)));
	
		c.position.set(newPos.x, newPos.y, newPos.z);
		c.position.needsUpdate = true;
	}

	XHX.GizmoMaterial = function ( parameters ) {

		THREE.MeshBasicMaterial.call( this );

		this.depthTest = true;
		this.depthWrite = false;
		this.side = THREE.DoubleSide;
		this.transparent = true;

		this.setValues( parameters );

		this.oldColor = this.color.clone();
		this.oldOpacity = this.opacity;
	};

	XHX.GizmoMaterial.prototype = Object.create( THREE.MeshBasicMaterial.prototype );
	XHX.GizmoMaterial.prototype.constructor = XHX.GizmoMaterial;

	var GizmoLineMaterial = function ( parameters ) {

		THREE.LineBasicMaterial.call( this );

		this.depthTest = true;
		this.depthWrite = false;
		this.transparent = true;
		this.linewidth = 1;

		this.setValues( parameters );

		this.oldColor = this.color.clone();
		this.oldOpacity = this.opacity;
	};

	GizmoLineMaterial.prototype = Object.create( THREE.LineBasicMaterial.prototype );
	GizmoLineMaterial.prototype.constructor = GizmoLineMaterial;

	function drawSquare(a, b, c, d) { 
		var square = new THREE.Geometry(); 

		//set 4 points
		square.vertices.push( a );
		square.vertices.push( b );
		square.vertices.push( c );
		square.vertices.push( d );

		//push 1 triangle
		square.faces.push( new THREE.Face3( 0,1,2) );

		//push another triangle
		square.faces.push( new THREE.Face3( 0,2,3) );

		//return the square object with BOTH faces
		return square;
	};

	XHX.Quad = function (c) {
		this.width = 1;
		this.height = 1;
		this.center = new THREE.Vector3(0, 0, 0);
		//only two are independent
		this.vx = new THREE.Vector3(1, 0, 0);
		this.vy = new THREE.Vector3(0, 1, 0);
		this.vz = new THREE.Vector3(0, 0, 1);

		this.planeMaterial = new XHX.GizmoMaterial( { color: c === undefined? 0xff00ff : c, opacity: 0.2 } );


		this.setSize = function(w, h) {
			this.width = w;
			this.height = h;
		}

		this.setBoundingBox = function(bb) {
			//determine bounding box in the xyplane
			var lp = this.globalToLocal(bb.min);
			var lowerX = lp.x;
			var upperX = lp.x;
			var lowerY = lp.y;
			var upperY = lp.y;
			lp = this.globalToLocal(bb.max);
			if (lowerX > lp.x) lowerX = lp.x;
			if (upperX < lp.x) upperX = lp.x;
			if (lowerY > lp.y) lowerY = lp.y;
			if (upperY < lp.y) upperY = lp.y;
			var p = new THREE.Vector3();
			p.set(bb.max.x, bb.min.y, bb.min.z);
			lp = this.globalToLocal(p);
			if (lowerX > lp.x) lowerX = lp.x;
			if (upperX < lp.x) upperX = lp.x;
			if (lowerY > lp.y) lowerY = lp.y;
			if (upperY < lp.y) upperY = lp.y;
			p.set(bb.max.x, bb.max.y, bb.min.z);
			lp = this.globalToLocal(p);
			if (lowerX > lp.x) lowerX = lp.x;
			if (upperX < lp.x) upperX = lp.x;
			if (lowerY > lp.y) lowerY = lp.y;
			if (upperY < lp.y) upperY = lp.y;
			p.set(bb.min.x, bb.max.y, bb.min.z);
			lp = this.globalToLocal(p);
			if (lowerX > lp.x) lowerX = lp.x;
			if (upperX < lp.x) upperX = lp.x;
			if (lowerY > lp.y) lowerY = lp.y;
			if (upperY < lp.y) upperY = lp.y;

			p.set(bb.min.x, bb.min.y, bb.max.z);
			lp = this.globalToLocal(p);
			if (lowerX > lp.x) lowerX = lp.x;
			if (upperX < lp.x) upperX = lp.x;
			if (lowerY > lp.y) lowerY = lp.y;
			if (upperY < lp.y) upperY = lp.y;
			p.set(bb.max.x, bb.min.y, bb.max.z);
			lp = this.globalToLocal(p);
			if (lowerX > lp.x) lowerX = lp.x;
			if (upperX < lp.x) upperX = lp.x;
			if (lowerY > lp.y) lowerY = lp.y;
			if (upperY < lp.y) upperY = lp.y;
			p.set(bb.min.x, bb.max.y, bb.max.z);
			lp = this.globalToLocal(p);
			if (lowerX > lp.x) lowerX = lp.x;
			if (upperX < lp.x) upperX = lp.x;
			if (lowerY > lp.y) lowerY = lp.y;
			if (upperY < lp.y) upperY = lp.y;

			var scale = 1.2;
			this.width = scale * (upperX - lowerX);
			this.height = scale * (upperY - lowerY);

			//adjust the center
			var lc = new THREE.Vector3(0.5 * (lowerX + upperX), 0.5 * (lowerY + upperY), 0.0);
			this.center = this.localToGlobal(lc);
		}

		this.setVxVy = function(vx, vy) {//assume vx and vy are vertical with each other
			vx.normalize();
			this.vx.copy(vx);
			vy.normalize();
			this.vy.copy(vy);

			this.vz = this.vx.clone().cross(vy);
		}

		this.setVxVz = function(vx, vz) {//assume vx and vz are vertical with each other
			vx.normalize();
			this.vx.copy(vx);
			vz.normalize();
			this.vz.copy(vz);

			this.vy = this.vz.clone().cross(vx);
		}

		this.localToGlobal = function(localP) {
			var globalP = this.center.clone();
			globalP.addScaledVector(this.vx, localP.x);
			globalP.addScaledVector(this.vy, localP.y);
			globalP.addScaledVector(this.vz, localP.z);

			return globalP;
		};

		this.globalToLocal = function(globalP) {
			var delta = globalP.clone().sub(this.center);

			var localP = new THREE.Vector3();
			localP.x = this.vx.dot(delta);
			localP.y = this.vy.dot(delta);
			localP.z = this.vz.dot(delta);

			return localP;
		};

		this.reverseVz = function() {
			this.vz.negate();
			var t = this.vx;
			this.vx = this.vy;
			this.vy = t;
		}
		
		this.apply = function() {
			if (this.msh !== undefined) {
				this.remove(this.msh);
				this.msh = undefined;
				this.geo = undefined;
			}
			//rebuild a new one
			var halfW = 0.5 * this.width;
			var halfH = 0.5 * this.height;

			//local points
			var la = new THREE.Vector3( -halfW, -halfH, 0);
			var lb = new THREE.Vector3(  halfW, -halfH, 0);
			var lc = new THREE.Vector3(  halfW,  halfH, 0);
			var ld = new THREE.Vector3( -halfW,  halfH, 0);
			
			//converted into globalP
			var ga = this.localToGlobal(la);
			var gb = this.localToGlobal(lb);
			var gc = this.localToGlobal(lc);
			var gd = this.localToGlobal(ld);

			this.geo = new drawSquare(ga, gb, gc, gd);
			this.msh = new THREE.Mesh( this.geo, this.planeMaterial );

			this.add( this.msh );
		}

		this.init = function () {

			THREE.Object3D.call( this );
			
			this.apply();
		};

		this.init();
	};

	XHX.Quad.prototype = Object.create( THREE.Object3D.prototype );
	XHX.Quad.prototype.constructor = XHX.Quad;
	
	XHX.Cone = function (loc, dir, size, color) {
		this.loc = loc;
		this.dir = dir;
		this.size = size;
		this.color = color;

		this.msh = undefined;
		this.geo = undefined;

		this.apply = function() {
			if (this.msh !== undefined) {
				this.remove(this.msh);
				this.msh = undefined;
				this.geo = undefined;
			}

            this.geo = new THREE.CylinderGeometry( 0.4 * this.size, 0, this.size, 6 );
			this.geo.translate( 0, 0.5 * this.size, 0 );
			this.geo.rotateX( Math.PI / 2 );
                
			this.msh = new THREE.Mesh( this.geo,  new XHX.GizmoMaterial( { color: this.color } ) );

            this.msh.position.set( 0, 0, 0 );
			this.msh.lookAt( this.dir );
			this.msh.position.copy( this.loc );

			this.add( this.msh );
		}

		this.init = function () {

			THREE.Object3D.call( this );
			
			this.apply();
		};

		this.init();
	};
	XHX.Cone.prototype = Object.create( THREE.Object3D.prototype );
	XHX.Cone.prototype.constructor = XHX.Cone;

	//dart = segment + cone
	XHX.Dart = function (color) {
		this.start = new THREE.Vector3(0, 0, 0);
		this.end = new THREE.Vector3(1, 0, 0);

		this.msh = undefined;
		this.geo = undefined;

		this.setEndPts = function(a, b) {
			this.start = a;
			this.end = b;
		};
		
		this.apply = function() {
			if (this.msh !== undefined) {
				this.remove(this.msh);
				this.msh = undefined;
				this.geo = undefined;
			}

			this.geo = new THREE.BufferGeometry();
			var seg = [this.start.x, this.start.y, this.start.z, this.end.x, this.end.y, this.end.z];
			this.geo.addAttribute( 'position', new THREE.Float32Attribute( seg, 3 ) );

			this.msh = new THREE.Line( this.geo, new GizmoLineMaterial( { color: 0xff0000 } ) );

			this.add( this.msh );
		}

		this.init = function () {

			THREE.Object3D.call( this );
			
			this.apply();
		};

		this.init();
	};
	XHX.Dart.prototype = Object.create( THREE.Object3D.prototype );
	XHX.Dart.prototype.constructor = XHX.Dart;

	XHX.StreamlineActor = function() {
		THREE.Object3D.call( this );			
		
		this.setTexture = function(t) {
			if (this.material === undefined)
				return;
			this.material.uniforms.texture1.value = t;
            this.material.uniforms.uHasResult.value = 1;
		}

	}
	XHX.StreamlineActor.prototype = Object.create( THREE.Object3D.prototype );
	XHX.StreamlineActor.prototype.constructor = XHX.StreamlineActor;

	/*
		1. A plane is added from the Add menu, in signals.objectSelected.add(), the plane is adjusted in size
		   according to study bounding box. The initial orientation is fixed at +z
		2. In transformControls.addEventListener( 'change' ), PlaneCtrl is created if it is null,
		   and editor.loader.xhxLoadCuttingPlane() is called to apply the cutting plane on the study.
		3. After the cuts are loaded, signals.xhxLoaded.add is called with case 6, to display the cut.
	*/
	XHX.CuttingPlaneActor = function() {
		THREE.Object3D.call( this );			
		
		this.enabled = false;
		this.plane = new THREE.Vector4(0.0, 0.0, 0.0, 0.0); //Ax + By + Cz = d, (A, B, C) is the normal
		this.material = undefined;
		this.cutHalfWidthScaleFactor = 1.0;
		this.glyphOnCutBothSides = false;

		this.str = function() {
			var sPlane = this.plane.x + ',' + this.plane.y 
				 + ',' + this.plane.z + ',' + this.plane.w;
			return sPlane;
		}

		this.setTexture = function(t) {
			if (this.material === undefined)
				return;
			this.material.uniforms.texture1.value = t;
            this.material.uniforms.uHasResult.value = 1;
		}

	}
	XHX.CuttingPlaneActor.prototype = Object.create( THREE.Object3D.prototype );
	XHX.CuttingPlaneActor.prototype.constructor = XHX.CuttingPlaneActor;

	XHX.IsoSurfaceActor = function() {
		THREE.Object3D.call( this );			
		
		//iso-surfaces
		this.clipValue = undefined; //single value clipping
		this.nContourCount = 5; //multiple clip values

		this.material = undefined;

		this.setTexture = function(t) {
			if (this.material !== undefined) {
                this.material.uniforms.texture1.value = t;
            }
            this.traverse( function ( object ) { 
                if (object.material !== undefined)
                    object.material.uniforms.texture1.value = t; 
            } );
		}

		this.isoMaterial = function(cv, texture) {
            var m;
            //each iso has to use a different material because its clip value (uniform variable) is different
            var uniforms = {
                "clipValue" :  { type: "f", value: cv},
                texture1: { type: 't', value: texture }
            };
            if (this.material === undefined) 
            {
                this.material = new THREE.ShaderMaterial({
                        uniforms: uniforms,
                        vertexShader: xhxRainbowShader.vertexShader_Iso,
                        fragmentShader: xhxRainbowShader.fragmentShader_Iso
                    });
                this.material.side = THREE.DoubleSide;//not default?

                m = this.material;
            } else {
                m = this.material.clone();
                m.uniforms = uniforms;
            }
			
			return m;
		}
	}
	XHX.IsoSurfaceActor.prototype = Object.create( THREE.Object3D.prototype );
	XHX.IsoSurfaceActor.prototype.constructor = XHX.IsoSurfaceActor;

	XHX.GlyphActor = function() {
		THREE.Object3D.call( this );			
		
		this.scaleFactor = 1.0;
	}
	XHX.GlyphActor.prototype = Object.create( THREE.Object3D.prototype );
	XHX.GlyphActor.prototype.constructor = XHX.GlyphActor;

	XHX.PartSurfaceActor = function() {
		THREE.Object3D.call( this );			

		this.texture; //the rain bow
		this.resultMaterial;
		this.edgeMaterial;
		this.transparentMaterial;

		this.getPart = function(idx) {
			var part;
			if (idx == this.children.length) {
				part = new THREE.Object3D();

				part.surfaceRltGroup = new THREE.Object3D();
				part.meshEdgeGroup = new THREE.Object3D();
				part.featureEdgeGroup = new THREE.Object3D();
				part.transparentGroup = new THREE.Object3D();

				part.add(part.surfaceRltGroup);
				part.add(part.meshEdgeGroup);
				part.add(part.featureEdgeGroup);
				part.add(part.transparentGroup);

				this.add(part);
			} else if (idx >= 0 && idx < this.children.length)
				part = this.children[idx];
			
			return part;
		}

		this.setTexture = function(t) {
			this.texture = t;
			if (this.resultMaterial !== undefined) {
            	this.resultMaterial.uniforms.texture1.value = t;
            	this.resultMaterial.uniforms.uHasResult.value = 1;
        	}
		}
	}
	XHX.PartSurfaceActor.prototype = Object.create( THREE.Object3D.prototype );
	XHX.PartSurfaceActor.prototype.constructor = XHX.PartSurfaceActor;

	//display info at top middle of the view
	XHX.rltName = "";
    XHX.studyName = "";
    XHX.infoUpdate = function(editor, infoStr, indpVals) {
      if (   XHX.studyName == editor.xhxDoc.study.info.name
          && XHX.rltName == infoStr) {
            //the info needs rebuild only if different result has been selected.
            //different studies may have the same name results, so, study name also needs to be checked.
            return; 
      }
      XHX.rltName = infoStr;
      XHX.studyName = editor.xhxDoc.study.info.name;

      if (infoStr == undefined)
        infoStr = "";
      var infoDiv = document.getElementById( 'infoA' );
      if (infoDiv == null) {
        infoDiv = document.createElement("Div");
        infoDiv.id = "infoA";
  
        var infoStyle = {
          position: "absolute",
          top: "10px",
          width: "100%",
          padding: "5px",
          textAlign: "center",
          zIndex: "8" //must lower than menu bar!
        };
  
        for (let style in infoStyle) {
          infoDiv.style[style] = infoStyle[style];
        }  
      } else {//remove all children, as new ones may need to be generated
        while(infoDiv.hasChildNodes())
        {
          infoDiv.removeChild(infoDiv.lastChild);
        }
      }
  
      if (infoDiv == undefined)
        return; //failed to find/create info element.
  
      if (indpVals != undefined && indpVals.length > 0) {//include independent variable
        infoStr += "<br>time: ";
        infoDiv.innerHTML = infoStr;//do this before combo is appended.
  
        //attach a combo box
        var combo = document.createElement("select");//a combo box
        combo.id = "infoCombo";
        infoDiv.appendChild(combo);
        for (var i = 0; i < indpVals.length; i++) {
          var option = document.createElement("option");
          option.id = i;
          option.text = indpVals[i];
          option.value = i;
          combo.add(option, null);
        }

        combo.onchange = function() {
          if (editor != undefined) {
            var study = editor.xhxDoc.study;
            var lSelectedIndpIdx = this.selectedIndex;
            editor.signals.xhxLoaded.dispatch(10, {'study':study, 'rltIndpIdx':lSelectedIndpIdx}); //9: load result at specified index      
          }
        };
      } else
        infoDiv.innerHTML = infoStr;
  
      document.getElementsByTagName("body")[0].appendChild(infoDiv);    
    }

}() );
