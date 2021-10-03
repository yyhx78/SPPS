
function init() {
    window.URL = window.URL || window.webkitURL;
    window.BlobBuilder = window.BlobBuilder || window.WebKitBlobBuilder || window.MozBlobBuilder;

    Number.prototype.format = function (){
        return this.toString().replace(/(\d)(?=(\d{3})+(?!\d))/g, "$1,");
    };

    //
    this.editor = new Editor();//this is global!

    var viewport = new Viewport( editor );
    document.body.appendChild( viewport.dom );

    //see link.js
    if (!IsInMode("openfoam"))
        loadLinkButtons();

    //
    createRenderer(editor, editor.config.getKey( 'project/renderer' ), 
                    editor.config.getKey( 'project/renderer/antialias' ), 
                    editor.config.getKey( 'project/renderer/shadows' ) );	

    //disable this for now
//    editor.setTheme( editor.config.getKey( 'theme' ) );

    showButtonBar();

};

//tjj: renderer, copied from the old Project tab. Should be moved to other places?
var rendererTypes = {
    'WebGLRenderer': THREE.WebGLRenderer,
    'SVGRenderer': THREE.SVGRenderer,
    'SoftwareRenderer': THREE.SoftwareRenderer,
    'RaytracingRenderer': THREE.RaytracingRenderer
};
	
var createRenderer = function(editor, type, antialias, shadows ) {

    var parameters = {};

    switch ( type ) {

        case 'WebGLRenderer':
            parameters.antialias = antialias;
            break;

        case 'RaytracingRenderer':
            parameters.workers = navigator.hardwareConcurrency || 4;
            parameters.workerPath = 'js/3JS/js/renderers/RaytracingWorker.js';
            parameters.randomize = true;
            parameters.blockSize = 64;
            break;
    }

    var renderer = new rendererTypes[ type ]( parameters );

    if ( shadows && renderer.shadowMap ) {

        renderer.shadowMap.enabled = true;
        // renderer.shadowMap.type = THREE.PCFSoftShadowMap;

    }

    editor.signals.rendererChanged.dispatch( renderer );

}		
