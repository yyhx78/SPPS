var objectControls;
var btns = [];
var btnCamera; //special camera for the buttons
var btnScene;

var loadLinkButtons = function()
{
    btnCamera = new THREE.PerspectiveCamera( 70, window.innerWidth / window.innerHeight, 1, 100000 );
    btnCamera.position.z = 600;
    btnScene = new THREE.Scene();

    objectControls = new ObjectControls( btnCamera );
    for( var i = 0; i < sm.length; i++ ) {
        var link = new Link( i , sm[i] );
        btns.push(link);
        objectControls.add( link.img );
        link.addToObject(btnScene);

        {
            link.scene.position.y = 2*60-( i * 60);
            link.scene.position.z = 200;
            link.scene.position.x = -320;
        }
    }
}


var sm = [

  {
    name: 'Twitter Share',
    note: 'c5',
    img:  'js/lib/img/icons/twitter_2.png',
    onClick: function () {
      fileInput.click();
    }
  },


  {
    name: 'Facebook',
    note: 'c5',
    img:  'js/lib/img/icons/facebook_2.png',
    onClick: function () {

    }
  },


   {
    name: 'Vimeo',
    note: 'c5',
    img:  'js/lib/img/icons/vimeo_2.png',
    onClick: function () {

    }
  },

  {
    name: 'Soundcloud',
    note: 'c5',
    img:  'js/lib/img/icons/soundcloud_2.png',
    onClick: function () {

    }
  },

  {
    name: 'Twitter',
    note: 'c5',
    img:  'js/lib/img/icons/cabbibo_2.png',
    onClick: function () {

    }
  }
]


function Link( id , params ){

    this.id = id;
    this.params = params;
    this.started = false;
  
    this.weight = Math.random() * .3 + .2;
    //this.weight = .1;
  
  
    this.time = { type:"f",value:0}
    this.startTime = { type:"f",value:0}
    
    this.info = { visible : false }
    
    this.sm =this.params.sm;
  
    {
      this.img = new THREE.Mesh(  
        new THREE.PlaneGeometry( 50 , 50 ),
        new THREE.MeshBasicMaterial({
          map: THREE.ImageUtils.loadTexture( this.params.img )
        })
      );
    }
  
    {
      this.img.material.opacity = .5;
      this.img.material.color = new THREE.Color( 1 , 1 , 1 );

      this.img.material.transparent = true;
      this.img.material.blending = THREE.AdditiveBlending;
      this.img.material.depthWrite = false;
      this.img.materialNeedsUpdate = true; 
    }
  
    this.img.hoverOver = this._hoverOver.bind( this );
    this.img.hoverOut  = this._hoverOut.bind( this );
    this.img.select    = this.select.bind( this );
    this.img.deselect  = this.deselect.bind( this );
  
  
    this.neutralColor = new THREE.Color( 0x444444 );
    this.focusColor   = new THREE.Color( 0xbbbbbb );
    this.hoveredColor = new THREE.Color( 0xffffff );
    
    this.scene = new THREE.Object3D();
    this.scene.add( this.img );  
  
  }
  
  
  Link.prototype.focus = function(){}
  Link.prototype.unFocus = function(){}
  Link.prototype.activate = function(){
   
  }
  Link.prototype.addToObject = function( obj ){
    obj.add(this.scene);
  }
  
  Link.prototype.deactivate = function(){
  
  
  }
  
  Link.prototype.tweenIn = function(){
  
  }
  
  
  Link.prototype.tweenOut = function(){
   
   
  }
  
  
  Link.prototype._hoverOver = function(){
  
    this.hoverOver();
  
  }
  
  Link.prototype.hoverOver = function( recursed ){
  
    this.img.material.opacity = 1;
    this.img.material.color = new THREE.Color( 1 , 1 , 0 );
    this.info.visible = true;
  
  if( this.screenshots ){
    for( var i =0; i < this.screenshots.length; i++ ){
      this.screenshots[i].visible = true;
    }
  }
  
  if( this.background ){ this.background.visible = true; }
  if( this.info ){ this.info.visible = true; 
    if( this.info.material){ this.info.material.uniforms.startTime.value = G.timer.value; }
  }
  
  }
  
  Link.prototype._hoverOut = function(){
  
    this.hoverOut();
    if( this.sm == false ){
      G.hoverOut( this.id , false );
    }
  
  }
  
  Link.prototype.hoverOut = function( recursed ){
  
    this.img.material.opacity = .5;
    this.img.material.color = new THREE.Color( 1 , 1 , 1 );

    //this.info.visible = false;
  
    if( this.screenshots ){
    for( var i =0; i < this.screenshots.length; i++ ){
      this.screenshots[i].visible = false;
    }
  }
  
  if( this.background ){ this.background.visible = false; }
  if( this.info ){ this.info.visible = false; }
  
  
  }
  
  Link.prototype.select = function(){
    
    if (this.params.onClick) {
      this.params.onClick();
    }
  
  }
  
  
  Link.prototype.deselect = function(){ 
  }
  
  
  var toScreenPosition = function(obj, camera)
  {
    var vector = new THREE.Vector3();

    var w = window.innerWidth;
    var h = window.innerHeight;
    
    var widthHalf = 0.5*w;//renderer.context.canvas.width;
    var heightHalf = 0.5*h;//renderer.context.canvas.height;

    obj.updateMatrixWorld();
    vector.setFromMatrixPosition(obj.matrixWorld);
    vector.project(camera);

    vector.x = ( vector.x * widthHalf ) + widthHalf;
    vector.y = - ( vector.y * heightHalf ) + heightHalf;

    return { 
      x: vector.x,
      y: vector.y
    };

  };

  var adjustBtnPosition = function(links, camera) {
    if (links.length > 0) {
      //adjust button position. There may be betteer way
      var delta = 0;
      var obj = links[0].scene;
      var pos = toScreenPosition(obj, camera);
      var leftMin = 45;
      var leftMax = 55;
      var max = 300; //incase infinite loop
      if (pos.x < leftMin) {
        do {
          if (pos.x < leftMin) {
            obj.position.x += 10;
            delta += 10;
            pos = toScreenPosition(obj, camera);
          }
          --max;
          if (max <= 0)
            break;
        } while (pos.x < leftMin);
      } else if (pos.x > leftMax) {
        do {
          if (pos.x > leftMax) {
            obj.position.x -= 10;
            delta -= 10;
            pos = toScreenPosition(obj, camera);
          }
          --max;
          if (max <= 0)
            break;
        } while (pos.x > leftMax);
      }

      if (delta != 0) {
        for( var i = 1; i < links.length; i++ ) {
          links[i].scene.position.x += delta;
        }
      }
    }
  }

  