var XHX = XHX || {};

XHX.Picker = function (aSignals, aXhxDoc, camera, domElement ) {
	THREE.Object3D.call( this );			

	// internals
    var scope = this;

    this.xhxDoc = aXhxDoc;
    var signals = aSignals;

    this.onCheckIntersection = undefined;

    this.deformedMeshGroup = undefined;
    this.meshGroup = undefined;

	this.camera = camera;
	this.domElement = ( domElement !== undefined ) ? domElement : document;

	this.mousePos = new THREE.Vector2();

    this.raycaster = new THREE.Raycaster();

    this.intersection = {
            intersects: false,
            face: new THREE.Face3(),
            point: new THREE.Vector3(),
            normal: new THREE.Vector3()
    };

	// API
	this.enabled = true; //call enable() at the end

    var cmdQueryStudy, cmdQueryStudyNext;
    this.queryStudy = function(aRayStart, aRayDirection, aCallBack) {
        if (aCallBack === undefined)
            return;

        var  study = this.xhxDoc.study;

        var fs = "" + aRayStart.x + "," + aRayStart.y + "," + aRayStart.z + ","
                    + aRayDirection.x + "," + aRayDirection.y + "," + aRayDirection.z;

        //2. load one study in the study list
        var sCmd = "?cmd=10&study=" + study.info.file + "&fs=" + fs;
        if (cmdQueryStudy == undefined || cmdQueryStudy.status == "returned") {
            cmdQueryStudy = {
                status: "running",
                cmdLine: sCmd
            };
            cmdQueryStudyNext = undefined;
        
            var loader = new THREE.xhxLoader();
            loader.load(sCmd, function (resp) {
                if (aCallBack !== undefined) {
                    aCallBack(resp); //the parameter is a function
                }

                cmdQueryStudy.status = "returned";
                
                if (cmdQueryStudyNext != undefined && cmdQueryStudyNext.status == "waiting") {
                    cmdQueryStudy = cmdQueryStudyNext;
                    cmdQueryStudy.status = "running";

                    var loader2 = new THREE.xhxLoader();
                    loader2.load(cmdQueryStudy.cmdLine, function (resp) {
                        if (aCallBack !== undefined) {
                            aCallBack(resp); //the parameter is a function
                        }
                        cmdQueryStudy.status = "returned";
                    });
                    cmdQueryStudyNext = undefined;                
                }
            });
        } else {
            cmdQueryStudyNext = {
                status : "waiting",
                cmdLine: sCmd
            };
        }
    };

    this.queryByServer = function() {
        scope.raycaster.setFromCamera( scope.mousePos, camera );
         var lRayOrg = scope.raycaster.ray.origin;
		 var lRayDir = scope.raycaster.ray.direction;

         this.queryStudy(lRayOrg, lRayDir, function(aRet) {
             var study = scope.xhxDoc.study;
            var iPos = 0;
    	    var iPartId = new Int32Array(aRet, iPos, 1); iPos += 4;
            if (iPartId[0] >= 0) {
    	        var iPieceId = new Int32Array(aRet, iPos, 1); iPos += 4;
    	        var iTriId = new Int32Array(aRet, iPos, 1); iPos += 4;
                
                var triNormal = new Float32Array(aRet, iPos, 3); iPos += 3 * 4;
                var hitPos = new Float32Array(aRet, iPos, 3); iPos += 3 * 4;
                var triXyz = new Float32Array(aRet, iPos, 3);

                var intersect = {};
                intersect.faceIndex = iTriId;
                intersect.point = new THREE.Vector3(hitPos[0], hitPos[1], hitPos[2]);
                intersect.distance = lRayOrg.distanceTo(intersect.point);

            	var part = study.data.parts[iPartId[0]];
				var triGeo = part.fragments[iPieceId[0]].triGeo;
                var idxArr = triGeo.index.array;

                var iPos = iTriId[0] * 3;
                intersect.face = new THREE.Face3(idxArr[iPos++], idxArr[iPos++], idxArr[iPos++], 
                                                 new THREE.Vector3(-triNormal[0], -triNormal[1], -triNormal[2]));

                var iPieceIdx = iPieceId[0];
                for (i = 0; i < iPartId[0]; i++) {
                    iPieceIdx += study.data.parts[i].fragments.length;
                }

                intersect.object = null;
                if (scope.meshGroup !== undefined)
                    intersect.object = scope.meshGroup.children[iPieceIdx];
                else if (scope.deformedMeshGroup !== undefined) 
                    intersect.object = scope.deformedMeshGroup.children[iPieceIdx];	

                if (intersect.object != null) {
                    scope.onIntersection(intersect);	
                    scope.intersection.intersects = true;
                }
            } else
                scope.intersection.intersects = false;
         })
    }

    this.onIntersection = function(intersect) {
        var study = scope.xhxDoc.study;

        var p = intersect.point;
        scope.intersection.point.copy( p );
        if (intersect.face)
            scope.intersection.face.copy(intersect.face);
        else
            scope.intersection.face = undefined;

        if (intersect.face)
        {
            var n = intersect.face.normal.clone();
            if (study !== undefined && study.data !== undefined)
                n.multiplyScalar(0.1 * study.data.size);
            else
                n.multiplyScalar( 1 );
            n.add( intersect.point );
            scope.intersection.normal.copy( intersect.face.normal );
        }

        if (intersect.object) {
            scope.intersection.object = intersect.object;
        }

        signals.xhxPickerEvent.dispatch(0, scope.intersection);
    }

    this.checkIntersection = function(objs) {
        if ( !objs ) 
            return;
        scope.raycaster.setFromCamera( scope.mousePos, camera );
        var intersects = scope.raycaster.intersectObjects( objs );
        if ( intersects.length > 0 ) {
            var intersect = intersects[ 0 ];
            this.onIntersection(intersect);
            scope.intersection.intersects = true;
        } else {
            scope.intersection.intersects = false;
        }

        if (this.onCheckIntersection !== undefined)
            this.onCheckIntersection(this);
    }

	this.onMouseDown = function( event ) {
		if ( scope.enabled === false ) return;
        event.preventDefault();
        
        scope.queryByServer();
	}

	this.onMouseMove = function( event ) {
		if ( scope.enabled === false ) return;
		event.preventDefault();

        var x, y;
        if ( event.changedTouches ) {
            x = event.changedTouches[ 0 ].pageX;
            y = event.changedTouches[ 0 ].pageY;
        } else {
            x = event.clientX;
            y = event.clientY;
        }    

        var rect = scope.domElement.getBoundingClientRect();

        var top = scope.top === undefined? rect.top : scope.top;
        scope.mousePos.x = ( (x - rect.left) / rect.width ) * 2 - 1;
        scope.mousePos.y = - ( (y - top) / rect.height ) * 2 + 1;
/* disable this, call  scope.queryByServer() in onMouseDown
        var bQueryByServer = true;
        if (bQueryByServer)
            scope.queryByServer();
        else if (scope.meshGroup !== undefined)
            scope.checkIntersection(scope.meshGroup.children);
        else if (scope.deformedMeshGroup !== undefined) 
            scope.checkIntersection(scope.deformedMeshGroup.children);		
*/
	}

	this.onMouseUp = function( event ) {
		if ( scope.enabled === false ) return;
    }
    
    this.enable = function(aEnable) {
        if (aEnable) {
            scope.domElement.addEventListener( 'contextmenu', function ( event ) { event.preventDefault(); }, false );
	        scope.domElement.addEventListener( 'mousedown', scope.onMouseDown, false );
	        scope.domElement.addEventListener( 'mousemove', scope.onMouseMove, false );
	        scope.domElement.addEventListener( 'mouseup', scope.onMouseUp, false );
        } else {
        	scope.domElement.removeEventListener( 'contextmenu', function ( event ) { event.preventDefault(); }, false );
	        scope.domElement.removeEventListener( 'mousedown', scope.onMouseDown, false );
	        scope.domElement.removeEventListener( 'mousemove', scope.onMouseMove, false );
	        scope.domElement.removeEventListener( 'mouseup', scope.onMouseUp, false );
        }

        scope.enabled = aEnable;
    }

    this.enable(true);
};

XHX.Picker.prototype = Object.create( THREE.Object3D.prototype );
Object.assign( XHX.Picker.prototype, THREE.EventDispatcher.prototype );
