var XHX = XHX || {};

/*
    var scene, camera, renderer;
    var zxGrid;

    var partSurfaceActor;
    var isoSurfaceActor;
    var LegendActor;
    var gnomeActor = new GnomeActor();
*/

( function () {
	'use strict';

	XHX.LegendActor = function() {
		THREE.Object3D.call( this );			

        this.sceneBackground;
        this.cameraBackground;
        this.rendererBackground;
        this.legendLabels;
        this.legendMesh;
        this.rainbowLUT;


        this.render = function() {
            if (this.rendererBackground !== undefined)
                this.rendererBackground.render(this.sceneBackground, this.cameraBackground);
        }
        /*
        Remember: to display the legend with a seperate camera and renderer, the other render must have { alpha: true }
                and a seperate container has to be used 
        */
        //build and display legend, it is used to set node color
        this.updateLegend = function(data) {
            if (this.sceneBackground === undefined) {
                this.sceneBackground = new THREE.Scene();

                this.cameraBackground = new THREE.PerspectiveCamera(50, window.innerWidth / window.innerHeight, 1, 10000);
                this.cameraBackground.position.z = 500;
                this.sceneBackground.add(this.cameraBackground);

                this.rendererBackground = new THREE.WebGLRenderer();
                this.rendererBackground.setClearColor( 0xffffff ); //background color
                this.rendererBackground.setSize(window.innerWidth, window.innerHeight);
                webglContainer.appendChild(this.rendererBackground.domElement);
            } else {
                if (this.legendMesh !== undefined) {
                    this.sceneBackground.remove ( this.legendMesh );
                    this.sceneBackground.remove ( this.legendLabels['title'] );
                    for ( var i = 0; i < Object.keys( this.legendLabels[ 'ticks' ] ).length; i++ ) {
                        this.sceneBackground.remove ( this.legendLabels[ 'ticks' ][ i ] );
                        this.sceneBackground.remove ( this.legendLabels[ 'lines' ][ i ] );
                    }
                }
            }

            //the name must match with the definition in XHXGui
            if (data.colorMap === undefined) {
                if (this.rainbowLUT === undefined)
                    colorMap = 'cooltowarm'; //default
                else
                    colorMap = this.rainbowLUT.mapname;//no change
            }
            else if (data.colorMap == 'Rainbow')
                colorMap = 'rainbow';
            else if (data.colorMap == 'Black body')
                colorMap = 'blackbody';
            else if (data.colorMap == 'Gray scale')
                colorMap = 'grayscale';
            else
                colorMap = 'cooltowarm';//should not happen

            var numberOfColors = 256;
            if (data.numberOfColors !== undefined) 
                numberOfColors = data.numberOfColors;
            else if (this.rainbowLUT !== undefined)
                numberOfColors = this.rainbowLUT.n;

            var p = this.cameraBackground.position;

            var pos = { 'x': 5.0, 'y': p.y, 'z': 490 };
            if (data.position === undefined && this.rainbowLUT !== undefined)
                pos = this.rainbowLUT.legend.position;

            var sLayout = 'vertical';
            if (data.orientation !== undefined)
                sLayout = data.orientation.toLowerCase();      
            else if (this.rainbowLUT !== undefined)
                sLayout = this.rainbowLUT.legend.layout;

            this.rainbowLUT = new THREE.Lut( colorMap, numberOfColors );

            this.legendMesh = this.rainbowLUT.setLegendOn( {'layout':sLayout,  'position': pos} );

            this.sceneBackground.add ( this.legendMesh );

            this.updateLegendLabels(data.title, 'Pa', 0.0, 1.0, 5);

            updateResultMaterialTexture(this.rainbowLUT.legend.texture);
        }

        this.updateLegendLabels = function(title, unit, minValue, maxValue, ticksCount) {
            if (this.sceneBackground === undefined || this.rainbowLUT ===undefined)
                return;

            if (this.legendLabels !== undefined) {
                if (title === undefined) {
                    title = this.legendLabels['title'];//use the old title                
                }
                this.sceneBackground.remove ( this.legendLabels['title'] );
                for ( var i = 0; i < Object.keys( this.legendLabels[ 'ticks' ] ).length; i++ ) {
                    this.sceneBackground.remove ( this.legendLabels[ 'ticks' ][ i ] );
                    this.sceneBackground.remove ( this.legendLabels[ 'lines' ][ i ] );
                }            
            }

            if (minValue == maxValue)
                maxValue += 1.0; //to display the contant as the minValue
            this.rainbowLUT.setMax(maxValue);
            this.rainbowLUT.setMin(minValue);

            if (title === undefined)
                title = 'Result';

            var notation;
            if (   Math.abs(minValue) < 1.0E-3 || Math.abs(minValue) > 1.0E5
                || Math.abs(maxValue) < 1.0E-3 || Math.abs(maxValue) > 1.0E5
                || (maxValue - minValue) < 0.1)
                notation = 'scientific';
            else
                notation = 'standard';

            this.legendLabels = this.rainbowLUT.setLegendLabels( { 'title': title, 'um': unit, 'ticks': ticksCount, 
                            'notation': notation, 'decimal': 3 } );

            this.sceneBackground.add ( this.legendLabels['title'] );
            for ( var i = 0; i < Object.keys( this.legendLabels[ 'ticks' ] ).length; i++ ) {
                this.sceneBackground.add ( this.legendLabels[ 'ticks' ][ i ] );
                this.sceneBackground.add ( this.legendLabels[ 'lines' ][ i ] );
            }
        }

	}
	XHX.LegendActor.prototype = Object.create( THREE.Object3D.prototype );
	XHX.LegendActor.prototype.constructor = XHX.LegendActor;
} )();

    function addLights() {
        var dirLight = new THREE.DirectionalLight(0xffffff, 1);
        dirLight.position.set(100, 100, 50);
        scene.add(dirLight);

        var ambLight = new THREE.AmbientLight(0x404040);
        scene.add(ambLight);
    }

	function onLegendNumberOfColorsChange(nColors) {
        if (LegendActor)
            LegendActor.updateLegend({'numberOfColors':nColors});   
    }

    function onBeforeIsoRender(aRenderer, aScene, aCamera, aGeometry, aMaterial, aGroup) {
        if (aGeometry.clipValue !== undefined) {
            aMaterial.uniforms.clipValue.value = aGeometry.clipValue;
            aMaterial.needsUpdate = true;
        }
    }

    function onBeforeCutRender(aRenderer, aScene, aCamera, aGeometry, aMaterial, aGroup) {
        if (aGeometry.clipValue !== undefined) {
            aMaterial.uniforms.clipValue.value = aGeometry.clipValue;
            aMaterial.needsUpdate = true;
        }
    }

function onWindowResize(){
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();
    gnomeActor.onWindowResize(camera.aspect);

    renderer.setSize( window.innerWidth, window.innerHeight );

    if (cameraBackground !== undefined) {
        cameraBackground.aspect = window.innerWidth / window.innerHeight;
        cameraBackground.updateProjectionMatrix();
    }
    if (rendererBackground !== undefined) {
        rendererBackground.setSize( window.innerWidth, window.innerHeight );
    }
}

function onMouseDown( event ) {
    if (picker.intersection.intersects) {
        var mshAttributes = partSurfaceActor.surfaceRltGroup.children[0].geometry.attributes;
    }
}

function onMouseUp( event ) {
}
function onMouseMove( event ) {
    var x, y;
    if ( event.changedTouches ) {
        x = event.changedTouches[ 0 ].pageX;
        y = event.changedTouches[ 0 ].pageY;
    } else {
        x = event.clientX;
        y = event.clientY;
    }    
}


function animate() {
    requestAnimationFrame(animate);
    render();
}

function render() {
    renderer.clear();
    renderer.setViewport(0, 0, window.innerWidth, window.innerHeight);
    renderer.setScissor(0, 0, window.innerWidth, window.innerHeight);
    renderer.render(scene, camera);
    renderer.clearDepth();

    gnomeActor.render(renderer);
    
    if (inProbeMode) {
        var left = 10;
        var right = 10;
        var width = window.innerWidth * 0.4;
        var height = window.innerHeight * 0.4;

        renderer.setViewport(left, right, width, height);
        renderer.setScissor(left, right, width, height);
        renderer.setClearColor(0x000000);
        renderer.render(gnomeActor.sceneGnome, gnomeActor.cameraGnome);
    }

    if (LegendActor)
        LegendActor.render();

    controls.update();
}

//functions copied from WebGL_Client/js/view.js
function addOption(comboBoxID, displayText, displayValue)
{
    var optionItem = document.createElement("option");
    
    optionItem.text = displayText;
    optionItem.value = displayValue;
    
    document.getElementById(comboBoxID).options.add(optionItem);
}

function onMeshCmptsCheckBoxClick(aChecked) {
    if (aChecked.checked)
        listContainer.appendChild(partListElement);
    else {
        listContainer.removeChild(partListElement);
    }
}

function onPlotOptionsCheckBoxClick(aChecked) {
    showPlotOptionsCtrl(aChecked.checked);
 }

function onCuttingPlaneCheckBoxClick(aChecked) {
    showCuttingPlaneCtrl(aChecked.checked);
 }

function onLegendSettingsCheckBoxClick(aChecked) {
    showLegendSettingsCtrl(aChecked.checked);
}

var cmmPlanes, cmmPlanesMsh;
function onDeformationCheckBoxClick(aChecked) {
    if (study !== undefined && study.data !== undefined) {
        if (cmmPlanes !== undefined) {
            scene.remove(cmmPlanes);
        }
    }

    if (aChecked.checked) {
        if (deformationElement === undefined)
            deformationGui(); //the settings dialog, it is in 'gui_container'
        else
            deformationContainer.appendChild(deformationElement);
    } else {
        if (deformationElement !== undefined)
            deformationContainer.removeChild(deformationElement);
    }
}

var inProbeMode = false;
function onProbeCheckBoxClick(aChecked) {
    inProbeMode = aChecked.checked;
    if (!inProbeMode)
        renderer.setClearColor(0xffffff); //restore the the default
}

function groupVisibilities(group, aVisibilities) {
    if (group !== undefined) {
        var children = group.children;
        for ( var i = 0, l = children.length; i < l; i ++ ) {
            children[i].visible = aVisibilities[i];
        }
    }    
}

function onMeshFragVisibilityChange(aVisibilities) {
    if (partSurfaceActor === undefined)
        return;

    if (aVisibilities === undefined || aVisibilities.length != study.data.parts.length)
        return;

    groupVisibilities(partSurfaceActor.surfaceRltGroup, aVisibilities);
    groupVisibilities(partSurfaceActor.meshEdgeGroup, aVisibilities);
    groupVisibilities(partSurfaceActor.featureEdgeGroup, aVisibilities);
    groupVisibilities(partSurfaceActor.transparentGroup, aVisibilities);
    groupVisibilities(isoSurfaceActor, aVisibilities);
    groupVisibilities(cuttingPlaneCtrl, aVisibilities);
}

function onResultSelectionChange(aNewRltIdx) {
    selectResult(aNewRltIdx);
}

function onLegendOrientationChanged( event ) {
    var data = event.detail;
    if (LegendActor)
        LegendActor.updateLegend(data);
}

function onLegendColorMapChanged( event ) {
    if (LegendActor)
        LegendActor.updateLegend(event.detail);
}

function onSurfaceVisibilityChange(aVisible) {    
    if (!partSurfaceActor)
        return;
    partSurfaceActor.surfaceRltGroup.traverse( function ( object ) { object.visible = aVisible; } );
}

function onMeshEdgesVisibilityChange(aVisible) {
    if (!aVisible) {
        if (partSurfaceActor.meshEdgeGroup !== undefined)
            partSurfaceActor.meshEdgeGroup.traverse( function ( object ) { object.visible = aVisible; } );
    } else {
        if (partSurfaceActor.meshEdgeGroup.children.length == 0) {
            loadMeshEdges();
        }
        if (partSurfaceActor.meshEdgeGroup !== undefined)
            partSurfaceActor.meshEdgeGroup.traverse( function ( object ) { object.visible = aVisible; } );
    }
}

function onContourPropertyChanged(aProperties) {
    showContours(aProperties);
}

function vDot(a, b) {
    var m = a.x * b.x + a.y * b.y + a.z * b.z;
    return m;
}

function onMinMaxAnimationPropertyChanged(aProperties) {

    var bSingleContour = aProperties.Enable;

    var rm = partSurfaceActor.getResultMaterial();

    if (!bSingleContour) {
        if (isoSurfaceActor !== undefined) {
            scene.remove(isoSurfaceActor);
            isoSurfaceActor.children = []; //clear all 
        }

        rm.uniforms.uSingleValueClipping.value = 0;

        return;
    }

    if (isoSurfaceActor === undefined)
        isoSurfaceActor = new XHX.isoSurfaceActor();

    var f2 = study.data.result.f2ScaleRange;

    //convert scale to value
    isoSurfaceActor.clipValue = f2[0] + (f2[1] - f2[0]) * (aProperties["Clip Value"] + 1.0)/2.0; //the range is [-1, 1]
    //convert to texture coordinates
    var t = (isoSurfaceActor.clipValue - f2[0]) / (f2[1] - f2[0]);

    rm.uniforms.uSingleValueClipping.value = 1;
    rm.uniforms.uClipValue.value = t;

    loadContours();
}

function onStudySelectionChanged(aStudyName) {

}

function onIndpValueChanged(iSelectedVal) {
    if (iRltIdx < 1 || iSelectedVal < 0)
      return; //no result displayed

    var rlt = study.data.results[iRltIdx - 1];
    if (rlt === undefined)
        return;
    
    if (rlt.indpVals === undefined || rlt.indpVals.length < 1 || iSelectedVal >= rlt.indpVals[0].length)
        return;

    if (iRltIndpIdx != iSelectedVal) {
      iRltIndpIdx = iSelectedVal;
      loadResult(study, iRltIdx - 1);
    }
}
