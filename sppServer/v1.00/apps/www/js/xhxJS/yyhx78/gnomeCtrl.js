//var XHX = XHX || {};

var GnomeActor = function() {
    this.sceneGnome = undefined;
    this.cameraGnome = undefined;

    this.onWindowResize = function(aspectRatio) {
        this.cameraGnome.aspect = aspectRatio;
        this.cameraGnome.updateProjectionMatrix();
    }

    this.init = function(aspectRatio, domE) {
        this.cameraGnome = new THREE.PerspectiveCamera(50, aspectRatio, 0.001, 10000);
        this.cameraGnome.position.z = 500;

        this.sceneGnome = new THREE.Scene();
        this.sceneGnome.add(this.cameraGnome);

        {
            var dirLight = new THREE.DirectionalLight(0xffffff, 1);
            dirLight.position.set(100, 100, 50);
            var ambLight = new THREE.AmbientLight(0x404040);

            this.sceneGnome.add(dirLight);
            this.sceneGnome.add(ambLight);
        }
        {
            var materialArray = [];

            var textureLoader = new THREE.TextureLoader();

            materialArray.push(new THREE.MeshBasicMaterial( { map: textureLoader.load( 'js/xhxJS/yyhx78/textures/front_x.png' ) })); //left
            materialArray.push(new THREE.MeshBasicMaterial( { map: textureLoader.load( 'js/xhxJS/yyhx78/textures/back_x.png' ) }));  //right
            materialArray.push(new THREE.MeshBasicMaterial( { map: textureLoader.load( 'js/xhxJS/yyhx78/textures/front_y.png' ) })); //top
            materialArray.push(new THREE.MeshBasicMaterial( { map: textureLoader.load( 'js/xhxJS/yyhx78/textures/back_y.png' ) }));  //bottom
            materialArray.push(new THREE.MeshBasicMaterial( { map: textureLoader.load( 'js/xhxJS/yyhx78/textures/front_z.png' ) })); //front
            materialArray.push(new THREE.MeshBasicMaterial( { map: textureLoader.load( 'js/xhxJS/yyhx78/textures/back_z.png' ) }));  //back
            var cubeGeom = new THREE.CubeGeometry( 100, 100, 100, 1, 1, 1 );
            cubeMesh = new THREE.Mesh( cubeGeom, new THREE.MeshFaceMaterial(materialArray) );
            cubeMesh.flipSided = true;
            cubeMesh.position.y = 10;
            this.sceneGnome.add( cubeMesh );
        }
        var gizmoMaterial = new THREE.MeshBasicMaterial( {
//            depthTest: false,
//            depthWrite: false,
//            transparent: true,
            side: THREE.DoubleSide,
            fog: false
        } );


	var gizmoLineMaterial = new THREE.LineBasicMaterial( {
//		depthTest: false,
//		depthWrite: false,
//		transparent: true,
		linewidth: 2,
		fog: false
	} );

    var matRed = gizmoMaterial.clone();
        matRed.color.set( 0xff0000 );
    
        var matGreen = gizmoMaterial.clone();
        matGreen.color.set( 0x00ff00 );
    
        var matBlue = gizmoMaterial.clone();
        matBlue.color.set( 0x0000ff );

        var matLineRed = gizmoLineMaterial.clone();
        matLineRed.color.set( 0xff0000 );
    
        var matLineGreen = gizmoLineMaterial.clone();
        matLineGreen.color.set( 0x00ff00 );
    
        var matLineBlue = gizmoLineMaterial.clone();
        matLineBlue.color.set( 0x0000ff );
    
        var arrowGeometry = new THREE.CylinderBufferGeometry( 0, 10, 50, 12, 100, false );

        var lineGeometry = new THREE.BufferGeometry( );
        lineGeometry.addAttribute( 'position', new THREE.Float32BufferAttribute( [ 0, 0, 0,	100, 0, 0 ], 3 ) );

        var gizmoTranslate = {
            X: [
                [ new THREE.Mesh( arrowGeometry, matRed ), [ 100, 0, 0 ], [ 0, 0, - Math.PI / 2 ], null, 'fwd' ],
//                [ new THREE.Mesh( arrowGeometry, matRed ), [ 100, 0, 0 ], [ 0, 0, Math.PI / 2 ], null, 'bwd' ],
                [ new THREE.Line( lineGeometry, matLineRed ) ]
            ],
            Y: [
                [ new THREE.Mesh( arrowGeometry, matGreen ), [ 0, 100, 0 ], null, null, 'fwd' ],
//                [ new THREE.Mesh( arrowGeometry, matGreen ), [ 0, 100, 0 ], [ Math.PI, 0, 0 ], null, 'bwd' ],
                [ new THREE.Line( lineGeometry, matLineGreen ), null, [ 0, 0, Math.PI / 2 ]]
            ],
            Z: [
                [ new THREE.Mesh( arrowGeometry, matBlue ), [ 0, 0, 100 ], [ Math.PI / 2, 0, 0 ], null, 'fwd' ],
//                [ new THREE.Mesh( arrowGeometry, matBlue ), [ 0, 0, 100 ], [ - Math.PI / 2, 0, 0 ], null, 'bwd' ],
                [ new THREE.Line( lineGeometry, matLineBlue ), null, [ 0, - Math.PI / 2, 0 ]]
            ]
        };
            
    	var setupGizmo = function ( gizmoMap ) {

	    	var gizmo = new THREE.Object3D();

		    for ( var name in gizmoMap ) {

                for ( var i = gizmoMap[ name ].length; i --; ) {

                    var object = gizmoMap[ name ][ i ][ 0 ].clone();
                    var position = gizmoMap[ name ][ i ][ 1 ];
                    var rotation = gizmoMap[ name ][ i ][ 2 ];
                    var scale = gizmoMap[ name ][ i ][ 3 ];
                    var tag = gizmoMap[ name ][ i ][ 4 ];
    
				// name and tag properties are essential for picking and updating logic.
    				object.name = name;
                    object.tag = tag;

                    if ( position ) {
                        object.position.set( position[ 0 ], position[ 1 ], position[ 2 ] );
                    }
                    if ( rotation ) {
                        object.rotation.set( rotation[ 0 ], rotation[ 1 ], rotation[ 2 ] );
                    }
                    if ( scale ) {
                        object.scale.set( scale[ 0 ], scale[ 1 ], scale[ 2 ] );
                    }
    
                    object.updateMatrix();
    
    				var tempGeometry = object.geometry.clone();
                    tempGeometry.applyMatrix( object.matrix );
                    object.geometry = tempGeometry;
                    object.renderOrder = Infinity;
    
                    object.position.set( 0, 0, 0 );
                    object.rotation.set( 0, 0, 0 );
                    object.scale.set( 1, 1, 1 );
    
                    gizmo.add( object );    
    			}
    		}

	    	return gizmo;
    	};

        var g = setupGizmo( gizmoTranslate );
        this.sceneGnome.add(g);
    }

    this.render = function(ren, rect) {
        if (rect === undefined) {
            var w = 0.25 * window.innerWidth;
            var h = 0.25 * window.innerHeight;
            ren.setViewport(window.innerWidth - w, window.innerHeight - h, w, h);
        } else {
            var width  = rect.right - rect.left;
            var height = rect.bottom - rect.top;

            var w = 0.25 * width;
            var h = 0.25 * height;
            ren.setViewport(width - w, height - h, w, h);
        }

        ren.render(this.sceneGnome, this.cameraGnome);
    }
}
