/**
 * @author mrdoob / http://mrdoob.com/
 */

var Loader = function ( editor, cb ) {

	var scope = this;
	var signals;
	if (editor != undefined)
		signals = editor.signals;

	this.texturePath = '';

	this.xhxLoadStudy = function(study) {
		if (study === undefined)
		   return;

		var studyLoader = new THREE.xhxLoader();
		var file = study.info.file;
		var sCmd = "?cmd=1&study=" + file;

		//a study data has just been loaded, destroy the old one and prepare to load a new one
		study.data = undefined;

		studyLoader.load(sCmd, function (project) {		

			editor.signals.xhxLoaded.dispatch(4, project); //4: project just loaded. Differences from 0: all parts are loaded

            study.data = {}; //reset the data, no touch on the info
            study.data.loaded = false;
            study.data.parts = project.parts;
            study.data.results = project.results;
			study.data.bbox = undefined;
			
            //3. load all parts (mesh components) in the study.
            var nLoadedParts = 0;
            for (var iPart in study.data.parts) {
                var partLoader = new THREE.xhxLoader(study);
                var sCmd = "?cmd=3&study=" + file + "&iPart=" + iPart;
                partLoader.load(sCmd, function (part) {
					nLoadedParts++;
                    if (study.data.bbox == undefined) {
                        study.data.bbox = new THREE.Box3();
                        study.data.bbox.copy(part.bbox);
                    } else
						study.data.bbox.union(part.bbox);

					editor.signals.xhxLoaded.dispatch(2, part); //2: study part loaded

					if (nLoadedParts == study.data.parts.length) {
						study.data.loaded = true;//all parts have been loaded
						//all parts have been loaded, this is supposed to be called only once
						editor.signals.xhxLoaded.dispatch(0, study); //0: study loaded
						editor.deselect(); //disable the selection to hide the translation/rotate/scale tool.	
					}
				});
			}
	    });
	};

	this.xhxLoadStudyList = function ( ) {
		if (editor.xhxDoc.studyList == undefined) {
		    //1. load study list
			var studiesLoader = new THREE.xhxLoader();
			var cmdLine = "?cmd=2";
			if (IsInMode("openfoam"))
				cmdLine += "&openfoam";//rebuild the list, new folders may have been added
    		studiesLoader.load(cmdLine, function (studyList) {
				editor.xhxDoc.studyList = [];
				var studies = editor.xhxDoc.studyList = [];
				for (i = 0; i < studyList.length; i++) {
		            var lStudy = {};
		            lStudy.info = studyList[i];
        		    studies[i] = lStudy;
				}
				
				if (studies.length > 0) {
					editor.signals.xhxLoaded.dispatch(1, studies); //1: study list loaded
					editor.xhxDoc.studyIdx = 0; //open the first by default
					editor.xhxDoc.study = studies[0];//mark the active study
					scope.xhxLoadStudy(editor.xhxDoc.study);
				}
		    });
		} else {
			var studies = editor.xhxDoc.studyList;
			editor.signals.xhxLoaded.dispatch(1, studies); //1: study list loaded
		}
	};

	this.xhxLoadMeshEdges = function(study, portion) {//1: mesh edges, 2: feature edges
		if (!study)
			return;

	    var rltLoader = new THREE.xhxLoader(study);

		for (var iPart in study.data.parts) {
			var cmdLine = "?cmd=8&study="
				+ study.info.file
				+ "&iPart=" + iPart
				+ "&iPortion=" + portion; //1: mesh edges
	
			rltLoader.load(cmdLine, function (result) {
				editor.signals.xhxLoaded.dispatch(8, {study:study, portion:portion}); //8: mesh edges loaded
			});
		}
	}

	this.xhxLoadGlyphs = function(study, cb) {
		if (!study)
			return;

	    var rltLoader = new THREE.xhxLoader(study);

		for (var iPart in study.data.parts) {
			var iRlt = study.data.resultIdx;
			var iRltIndp = study.data.resultIndpIdx;
					
			var cmdLine = "?cmd=11&study="
				+ study.info.file
				+ "&iPart=" + iPart
				+ "&iRlt=" + iRlt
				+ "&iRltIndp=" + iRltIndp;
	
			rltLoader.load(cmdLine, function (result) {
				if (cb != undefined)
					cb(result);
		});
		}
	}

	var cmdLoadResult, cmdLoadResultNext;
	this.xhxLoadResult = function(aStudy, aRltIdx, aRltIndpIdx) {
		if (!aStudy)
			return;

		var rlt = aStudy.data.results[aRltIdx];
		if (rlt === undefined)
			return;
	
		var iRltIndpIdx = aRltIndpIdx;
		if (rlt.indpVals !== undefined && rlt.indpVals.length > 0) {
			if (iRltIndpIdx < 0 || iRltIndpIdx >= rlt.indps.length) {
				//by default, take the last one. Only single indp is considered for now (the first of indp array)
				iRltIndpIdx = 0;//rlt.indpVals[0].length - 1;
			}
		} else
			iRltIndpIdx = -1;
	
		var sCmd = "?cmd=4&study=" + aStudy.info.file + "&iRlt=" + aRltIdx + "&iIndp=" + iRltIndpIdx;
	
		if (cmdLoadResult == undefined || cmdLoadResult.status == "returned") {
			cmdLoadResult = {
				status: "running",
				cmdLine: sCmd
			};
			cmdLoadResultNext = undefined;
	
			var rltLoader = new THREE.xhxLoader(aStudy);
			rltLoader.load(sCmd, function (result) {
				aStudy.data.result = result; //result values on each mesh fragments
				aStudy.data.resultIdx = aRltIdx;
				aStudy.data.resultIndpIdx = aRltIndpIdx;
	
				//recreate cutting plane to catch the color changes
				signals.xhxLoaded.dispatch(3, aStudy); //3: result loaded (result has been hold in the study data as shown above)
	
				cmdLoadResult.status = "returned";
	
				if (cmdLoadResultNext != undefined && cmdLoadResultNext.status == "waiting") {
					cmdLoadResult = cmdLoadResultNext;
					cmdLoadResult.status = "running";
	
					var rltLoader2 = new THREE.xhxLoader();
					rltLoader2.load(sCmd, function (result) {
						aStudy.data.result = result; //result values on each mesh fragments
						aStudy.data.resultIdx = aRltIdx;
	
						//recreate cutting plane to catch the color changes
						scope.xhxLoadCuttingPlane();
						signals.xhxLoaded.dispatch(3, aStudy); //3: result loaded (result has been hold in the study data as shown above)
	
						cmdLoadResult.status = "returned";
					});
	
					cmdLoadResultNext = undefined;                
				}
			});
		}
	}
	

	var xhxPlaneStr = function(plane) {
		var sPlane = plane.x + ',' + plane.y 
			 + ',' + plane.z + ',' + plane.w;
		return sPlane;
	}
	var cmdLoadCuttingPlane, cmdLoadCuttingPlaneNext;
	this.xhxLoadCuttingPlane = function(study, plane) {
		if (study === undefined || study.data === undefined || !study.data.loaded)
			return;
		
		if (plane === undefined) //the plane used to cut
			return;
				
		var iRltId = -1;
		if (study.data.result !== undefined)// && xhxGetResultMaterial().uniforms.uHasResult.value == 1) //result exists but may not be used
			iRltId = study.data.resultIdx;
		
		//cut on volume
		var sPlane = xhxPlaneStr(plane);
		
		var cmdLine = "?cmd=5&study=" + study.info.file + "&plane=" + sPlane + "&iRlt=" + iRltId;
		
		if (cmdLoadCuttingPlane == undefined || cmdLoadCuttingPlane.status == "returned") {
			cmdLoadCuttingPlane = {
				status: "running",
				cmdLine: cmdLine
			};
			cmdLoadCuttingPlaneNext = undefined;
			var rltLoader = new THREE.xhxLoader(study);
			rltLoader.load(cmdLine, function (result) {

				editor.signals.xhxLoaded.dispatch(6, result.parts); //6: cutting plane loaded
		
				cmdLoadCuttingPlane.status = "returned";
	
				if (cmdLoadCuttingPlaneNext != undefined && cmdLoadCuttingPlaneNext.status == "waiting") {
					cmdLoadCuttingPlane = cmdLoadCuttingPlaneNext;
					cmdLoadCuttingPlane.status = "running";
					var rltLoader2 = new THREE.xhxLoader(study);
					rltLoader2.load(cmdLoadCuttingPlane.cmdLine, function (result) {

						editor.signals.xhxLoaded.dispatch(6, result.parts); //6: cutting plane loaded
	
						cmdLoadCuttingPlane.status = "returned";
					});
	
					cmdLoadCuttingPlaneNext = undefined;                
				}
			});
		} else {
			cmdLoadCuttingPlaneNext = {
				status : "waiting",
				cmdLine: cmdLine
			};
		}		
	}

	var cmdLoadContours, cmdLoadContoursNext;
	this.xhxLoadContours = function(study, fIso, value) {//fIso: value clip or number of contours
	    if (study === undefined || study.data === undefined || !study.data.loaded)
    	    return;

	    if (study.data.result === undefined)
    	    return;//a result must have been loaded.

	    var iRltId = study.data.resultIdx;

	    var cmdLine = "?cmd=6&study=" + study.info.file;
    	if (fIso)
        	cmdLine += "&fIso=" + value + "&iRlt=" + iRltId;
	    else
    	    cmdLine += "&nIso=" + value + "&iRlt=" + iRltId;

    	if (cmdLoadContours == undefined || cmdLoadContours.status == "returned") {
        	cmdLoadContours = {
            	status: "running",
    	        cmdLine: cmdLine
        	};
        	cmdLoadContoursNext = undefined;

	        var rltLoader = new THREE.xhxLoader();
			rltLoader.load(cmdLine, function (result) {
				editor.signals.xhxLoaded.dispatch(7, result.parts); //6: iso-surfaces loaded
		
				cmdLoadContours.status = "returned";
	
				if (cmdLoadContoursNext != undefined && cmdLoadContoursNext.status == "waiting") {
					cmdLoadContours = cmdLoadContoursNext;
					cmdLoadContours.status = "running";
					var rltLoader2 = new THREE.xhxLoader();
					rltLoader2.load(cmdLoadContours.cmdLine, function (result) {
						editor.signals.xhxLoaded.dispatch(7, result.parts); //6: iso-surfaces loaded
	
						cmdLoadContours.status = "returned";
					});
	
					cmdLoadContoursNext = undefined;                
				}
			});
		} else {
	        cmdLoadContoursNext = {
    	        status : "waiting",
        	    cmdLine: cmdLine
        	};
    	}
	}

	this.loadFiles = function ( files ) {

		if ( files.length > 0 ) {

			var filesMap = createFileMap( files );

			var manager = new THREE.LoadingManager();
			manager.setURLModifier( function ( url ) {

				var file = filesMap[ url ];

				if ( file ) {

					console.log( 'Loading', url );

					return URL.createObjectURL( file );

				}

				return url;

			} );

			for ( var i = 0; i < files.length; i ++ ) {

				scope.loadFile( files[ i ], manager );

			}

		}

	};

	this.loadFile = function ( file, manager ) {

		var filename = file.name;
		var extension = filename.split( '.' ).pop().toLowerCase();

		var reader = new FileReader();
		reader.addEventListener( 'progress', function ( event ) {

//			var size = '(' + Math.floor( event.total / 1000 ).format() + ' KB)';
//			var progress = Math.floor( ( event.loaded / event.total ) * 100 ) + '%';

//			console.log( 'Loading', filename, size, progress );

		} );

		switch ( extension ) {

			case '3ds':

				reader.addEventListener( 'load', function ( event ) {

					var loader = new THREE.TDSLoader();
					var object = loader.parse( event.target.result );

					editor.execute( new AddObjectCommand( editor, object ) );

				}, false );
				reader.readAsArrayBuffer( file );

				break;

			case 'amf':

				reader.addEventListener( 'load', function ( event ) {

					var loader = new THREE.AMFLoader();
					var amfobject = loader.parse( event.target.result );

					editor.execute( new AddObjectCommand( editor, amfobject ) );

				}, false );
				reader.readAsArrayBuffer( file );

				break;

			case 'awd':

				reader.addEventListener( 'load', function ( event ) {

					var loader = new THREE.AWDLoader();
					var scene = loader.parse( event.target.result );

					editor.execute( new SetSceneCommand( editor, scene ) );

				}, false );
				reader.readAsArrayBuffer( file );

				break;

			case 'babylon':

				reader.addEventListener( 'load', function ( event ) {

					var contents = event.target.result;
					var json = JSON.parse( contents );

					var loader = new THREE.BabylonLoader();
					var scene = loader.parse( json );

					editor.execute( new SetSceneCommand( editor, scene ) );

				}, false );
				reader.readAsText( file );

				break;

			case 'babylonmeshdata':

				reader.addEventListener( 'load', function ( event ) {

					var contents = event.target.result;
					var json = JSON.parse( contents );

					var loader = new THREE.BabylonLoader();

					var geometry = loader.parseGeometry( json );
					var material = new THREE.MeshStandardMaterial();

					var mesh = new THREE.Mesh( geometry, material );
					mesh.name = filename;

					editor.execute( new AddObjectCommand( editor, mesh ) );

				}, false );
				reader.readAsText( file );

				break;

			case 'dae':

				reader.addEventListener( 'load', function ( event ) {

					var contents = event.target.result;

					var loader = new THREE.ColladaLoader( manager );
					var collada = loader.parse( contents );

					collada.scene.name = filename;

					editor.addAnimation( collada.scene, collada.animations );
					editor.execute( new AddObjectCommand( editor, collada.scene ) );

				}, false );
				reader.readAsText( file );

				break;

			case 'fbx':

				reader.addEventListener( 'load', function ( event ) {

					var contents = event.target.result;

					var loader = new THREE.FBXLoader( manager );
					var object = loader.parse( contents );

					editor.addAnimation( object, object.animations );
					editor.execute( new AddObjectCommand( editor, object ) );

				}, false );
				reader.readAsArrayBuffer( file );

				break;

			case 'glb':

				reader.addEventListener( 'load', function ( event ) {

					var contents = event.target.result;

					var dracoLoader = new THREE.DRACOLoader();
					dracoLoader.setDecoderPath( 'js/3JS/js/libs/draco/gltf/' );

					var loader = new THREE.GLTFLoader();
					loader.setDRACOLoader( dracoLoader );
					loader.parse( contents, '', function ( result ) {

						var scene = result.scene;
						scene.name = filename;

						editor.addAnimation( scene, result.animations );
						editor.execute( new AddObjectCommand( editor, scene ) );

					} );

				}, false );
				reader.readAsArrayBuffer( file );

				break;

			case 'gltf':

				reader.addEventListener( 'load', function ( event ) {

					var contents = event.target.result;

					var loader;

					if ( isGLTF1( contents ) ) {

						loader = new THREE.LegacyGLTFLoader( manager );

					} else {

						loader = new THREE.GLTFLoader( manager );

					}

					loader.parse( contents, '', function ( result ) {

						var scene = result.scene;
						scene.name = filename;

						editor.addAnimation( scene, result.animations );
						editor.execute( new AddObjectCommand( editor, scene ) );

					} );

				}, false );
				reader.readAsArrayBuffer( file );

				break;

			case 'js':
			case 'json':

			case '3geo':
			case '3mat':
			case '3obj':
			case '3scn':

				reader.addEventListener( 'load', function ( event ) {

					var contents = event.target.result;

					// 2.0

					if ( contents.indexOf( 'postMessage' ) !== - 1 ) {

						var blob = new Blob( [ contents ], { type: 'text/javascript' } );
						var url = URL.createObjectURL( blob );

						var worker = new Worker( url );

						worker.onmessage = function ( event ) {

							event.data.metadata = { version: 2 };
							handleJSON( event.data, file, filename );

						};

						worker.postMessage( Date.now() );

						return;

					}

					// >= 3.0

					var data;

					try {

						data = JSON.parse( contents );

					} catch ( error ) {

						alert( error );
						return;

					}

					handleJSON( data, file, filename );

				}, false );
				reader.readAsText( file );

				break;


			case 'kmz':

				reader.addEventListener( 'load', function ( event ) {

					var loader = new THREE.KMZLoader();
					var collada = loader.parse( event.target.result );

					collada.scene.name = filename;

					editor.execute( new AddObjectCommand( editor, collada.scene ) );

				}, false );
				reader.readAsArrayBuffer( file );

				break;

			case 'md2':

				reader.addEventListener( 'load', function ( event ) {

					var contents = event.target.result;

					var geometry = new THREE.MD2Loader().parse( contents );
					var material = new THREE.MeshStandardMaterial( {
						morphTargets: true,
						morphNormals: true
					} );

					var mesh = new THREE.Mesh( geometry, material );
					mesh.mixer = new THREE.AnimationMixer( mesh );
					mesh.name = filename;

					editor.addAnimation( mesh, geometry.animations );
					editor.execute( new AddObjectCommand( editor, mesh ) );

				}, false );
				reader.readAsArrayBuffer( file );

				break;

			case 'obj':

				reader.addEventListener( 'load', function ( event ) {

					var contents = event.target.result;

					var object = new THREE.OBJLoader().parse( contents );
					object.name = filename;

					editor.execute( new AddObjectCommand( editor, object ) );

				}, false );
				reader.readAsText( file );

				break;

			case 'playcanvas':

				reader.addEventListener( 'load', function ( event ) {

					var contents = event.target.result;
					var json = JSON.parse( contents );

					var loader = new THREE.PlayCanvasLoader();
					var object = loader.parse( json );

					editor.execute( new AddObjectCommand( editor, object ) );

				}, false );
				reader.readAsText( file );

				break;

			case 'ply':

				reader.addEventListener( 'load', function ( event ) {

					var contents = event.target.result;

					var geometry = new THREE.PLYLoader().parse( contents );
					geometry.sourceType = "ply";
					geometry.sourceFile = file.name;

					var material = new THREE.MeshStandardMaterial();

					var mesh = new THREE.Mesh( geometry, material );
					mesh.name = filename;

					editor.execute( new AddObjectCommand( editor, mesh ) );

				}, false );
				reader.readAsArrayBuffer( file );

				break;

			case 'stl':

				reader.addEventListener( 'load', function ( event ) {

					var contents = event.target.result;

					var geometry = new THREE.STLLoader().parse( contents );
					geometry.sourceType = "stl";
					geometry.sourceFile = file.name;

					if (editor != undefined) {
						var material = new THREE.MeshStandardMaterial();
						var mesh = new THREE.Mesh( geometry, material );
						mesh.name = filename;
						editor.execute( new AddObjectCommand( editor, mesh ) );
					}
					else if (cb != undefined)
						cb(geometry);

				}, false );

				if ( reader.readAsBinaryString !== undefined ) {

					reader.readAsBinaryString( file );

				} else {

					reader.readAsArrayBuffer( file );

				}

				break;

			case 'svg':

				reader.addEventListener( 'load', function ( event ) {

					var contents = event.target.result;

					var loader = new THREE.SVGLoader();
					var paths = loader.parse( contents ).paths;

					//

					var group = new THREE.Group();
					group.scale.multiplyScalar( 0.1 );
					group.scale.y *= - 1;

					for ( var i = 0; i < paths.length; i ++ ) {

						var path = paths[ i ];

						var material = new THREE.MeshBasicMaterial( {
							color: path.color,
							depthWrite: false
						} );

						var shapes = path.toShapes( true );

						for ( var j = 0; j < shapes.length; j ++ ) {

							var shape = shapes[ j ];

							var geometry = new THREE.ShapeBufferGeometry( shape );
							var mesh = new THREE.Mesh( geometry, material );

							group.add( mesh );

						}

					}

					editor.execute( new AddObjectCommand( editor, group ) );

				}, false );
				reader.readAsText( file );

				break;

			case 'vtk':

				reader.addEventListener( 'load', function ( event ) {

					var contents = event.target.result;

					var geometry = new THREE.VTKLoader().parse( contents );
					geometry.sourceType = "vtk";
					geometry.sourceFile = file.name;

					var material = new THREE.MeshStandardMaterial();

					var mesh = new THREE.Mesh( geometry, material );
					mesh.name = filename;

					editor.execute( new AddObjectCommand( editor, mesh ) );

				}, false );
				reader.readAsText( file );

				break;

			case 'wrl':

				reader.addEventListener( 'load', function ( event ) {

					var contents = event.target.result;

					var result = new THREE.VRMLLoader().parse( contents );

					editor.execute( new SetSceneCommand( editor, result ) );

				}, false );
				reader.readAsText( file );

				break;

			case 'zip':

				reader.addEventListener( 'load', function ( event ) {

					handleZIP( event.target.result );

				}, false );
				reader.readAsBinaryString( file );

				break;

			case 'foam':

				reader.addEventListener( 'load', function ( event ) {

					var study = editor.xhxDoc.study;
					study.info.file = event.target.result;
					editor.loader.xhxLoadStudy(study);

				}, false );

				reader.readAsText( file );
				
				break;

			default:

				// alert( 'Unsupported file format (' + extension +  ').' );

				break;

		}

	};

	function handleJSON( data, file, filename ) {

		if ( data.metadata === undefined ) { // 2.0

			data.metadata = { type: 'Geometry' };

		}

		if ( data.metadata.type === undefined ) { // 3.0

			data.metadata.type = 'Geometry';

		}

		if ( data.metadata.formatVersion !== undefined ) {

			data.metadata.version = data.metadata.formatVersion;

		}

		switch ( data.metadata.type.toLowerCase() ) {

			case 'buffergeometry':

				var loader = new THREE.BufferGeometryLoader();
				var result = loader.parse( data );

				var mesh = new THREE.Mesh( result );

				editor.execute( new AddObjectCommand( editor, mesh ) );

				break;

			case 'geometry':

				console.error( 'Loader: "Geometry" is no longer supported.' );

				break;

			case 'object':

				var loader = new THREE.ObjectLoader();
				loader.setResourcePath( scope.texturePath );

				var result = loader.parse( data );

				if ( result.isScene ) {

					editor.execute( new SetSceneCommand( editor, result ) );

				} else {

					editor.execute( new AddObjectCommand( editor, result ) );

				}

				break;

			case 'app':

				editor.fromJSON( data );

				break;

		}

	}

	function createFileMap( files ) {

		var map = {};

		for ( var i = 0; i < files.length; i ++ ) {

			var file = files[ i ];
			map[ file.name ] = file;

		}

		return map;

	}

	function handleZIP( contents ) {

		var zip = new JSZip( contents );

		// Poly

		if ( zip.files[ 'model.obj' ] && zip.files[ 'materials.mtl' ] ) {

			var materials = new THREE.MTLLoader().parse( zip.file( 'materials.mtl' ).asText() );
			var object = new THREE.OBJLoader().setMaterials( materials ).parse( zip.file( 'model.obj' ).asText() );
			editor.execute( new AddObjectCommand( editor, object ) );

		}

		//

		zip.filter( function ( path, file ) {

			var manager = new THREE.LoadingManager();
			manager.setURLModifier( function ( url ) {

				var file = zip.files[ url ];

				if ( file ) {

					console.log( 'Loading', url );

					var blob = new Blob( [ file.asArrayBuffer() ], { type: 'application/octet-stream' } );
					return URL.createObjectURL( blob );

				}

				return url;

			} );

			var extension = file.name.split( '.' ).pop().toLowerCase();

			switch ( extension ) {

				case 'fbx':

					var loader = new THREE.FBXLoader( manager );
					var object = loader.parse( file.asArrayBuffer() );

					editor.execute( new AddObjectCommand( editor, object ) );

					break;

				case 'glb':

					var loader = new THREE.GLTFLoader();
					loader.parse( file.asArrayBuffer(), '', function ( result ) {

						var scene = result.scene;

						editor.addAnimation( scene, result.animations );
						editor.execute( new AddObjectCommand( editor, scene ) );

					} );

					break;

				case 'gltf':

					var loader = new THREE.GLTFLoader( manager );
					loader.parse( file.asText(), '', function ( result ) {

						var scene = result.scene;

						editor.addAnimation( scene, result.animations );
						editor.execute( new AddObjectCommand( scene ) );

					} );

					break;

			}

		} );

	}

	function isGLTF1( contents ) {

		var resultContent;

		if ( typeof contents === 'string' ) {

			// contents is a JSON string
			resultContent = contents;

		} else {

			var magic = THREE.LoaderUtils.decodeText( new Uint8Array( contents, 0, 4 ) );

			if ( magic === 'glTF' ) {

				// contents is a .glb file; extract the version
				var version = new DataView( contents ).getUint32( 4, true );

				return version < 2;

			} else {

				// contents is a .gltf file
				resultContent = THREE.LoaderUtils.decodeText( new Uint8Array( contents ) );

			}

		}

		var json = JSON.parse( resultContent );

		return ( json.asset != undefined && json.asset.version[ 0 ] < 2 );

	}

};
