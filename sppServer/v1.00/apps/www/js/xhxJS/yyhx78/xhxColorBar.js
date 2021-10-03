var XHX = XHX || {};

( function () {

	'use strict';

	XHX.ColorBar = function ( parameters ) {
		// look up the text canvas.
		var textCanvas = document.getElementById("text");
		// make a 2D context for it
		var ctx = textCanvas? textCanvas.getContext("2d") : undefined;

		var visible = 0; //not visible by default

		var fontSize = 12;
		ctx.font = '12px serif';

		var title = "mm";
		var minValue = 0.0, maxValue = 10;
		var tickLength = 5;

		var colorMapName = 'rainbow';
		var ctxNumberOfColors = 256;
		var ctxLeft = 100, ctxTop = 80;
		var ctxLut = new THREE.LutInOld3JS(colorMapName, ctxNumberOfColors);

		var orientation = 0; //0: horizontal, 1: vertical
		var labelOnLeft = 1; //0: left/top, 1: right/bottom
		var ctxWidth = 356, ctxHeight = 25;
		if (orientation !== 0) {
			ctxWidth = 25;
			ctxHeight = 300;
		}

		var numberOfLabels = 5;

		this.setTitle = function(t) {
			title = t;
		}

		this.setVisible = function(i) {
			visible = i;
		}

		this.setPosition = function(left, top) {
			ctxLeft = left;
			ctxTop = top;
		}

		this.getWidth = function() {
			return ctxWidth;
		}

		this.getHeight = function() {
			return ctxHeight;
		}
		
		this.setColorMapName = function(name) {
			colorMapName = name;
			//rebuild the color table
			ctxLut = new THREE.LutInOld3JS(colorMapName, ctxNumberOfColors);
			//the texture will also need to be updated
		}

		this.setNumberOfColors = function(n) {
			ctxNumberOfColors = n;
			//rebuild the color table
			ctxLut = new THREE.LutInOld3JS(colorMapName, ctxNumberOfColors);
			//the texture will also need to be updated
		}

		this.setScalarRange = function(l, h) {
			minValue = l;
			maxValue = h;
		}

		//create a 1d texture according to the LUT (match with the color bar)
		this.createTexture = function() {
			var textureSizeX = ctxNumberOfColors;
			var textureSizeY = 1;
			var size = textureSizeX * textureSizeY;
			var data = new Uint8Array( size * 3 ); 

			for (var i = 0; i < ctxNumberOfColors; i++) {
				var colorValue = (1.0 * i)/(ctxNumberOfColors - 1);
				var color = ctxLut.getColor(colorValue);

				var r = Math.floor( color.r * 255 );
				var g = Math.floor( color.g * 255 );
				var b = Math.floor( color.b * 255 );
				
				var stride = i * 3;
				
				data[ stride ] = r;
				data[ stride + 1 ] = g;
				data[ stride + 2 ] = b;
			}

			var texture = new THREE.DataTexture( data, textureSizeX, textureSizeY, THREE.RGBFormat ); 
			texture.minFilter = THREE.NearestFilter; 
			texture.magFilter = THREE.NearestFilter; 
			texture.needsUpdate = true; 

			return texture;
		}

		this.render = function () {
			if (!ctx)
			   return;
			// Clear the 2D canvas
			ctx.clearRect(0, 0, ctx.canvas.width, ctx.canvas.height);

			if (visible != 1)
				return;

			//color rectangle
			var nSteps = orientation === 0? ctxWidth : ctxHeight;
			for (var i = 0; i < nSteps; i++) {
			  	var colorValue = (1.0 * i)/(nSteps - 1);
			  	if (orientation != 0)
			     	colorValue = 1.0 - colorValue;
			  	var color = ctxLut.getColor(colorValue);
			  	ctx.strokeStyle = 'rgb(' + color.r * 255 + ', ' + color.g * 255 + ',' + color.b * 255 + ')';

				ctx.beginPath();  // Start a new path
				if (orientation === 0) {
					ctx.moveTo(i + ctxLeft, ctxTop);
					ctx.lineTo(i + ctxLeft, ctxTop + ctxHeight);
				}
			  	else {
					ctx.moveTo(ctxLeft, i + ctxTop + ctxHeight);
					ctx.lineTo(ctxLeft + ctxWidth, i + ctxTop + ctxHeight);
				}
				ctx.stroke();          // Render the path
			}

			ctx.fillStyle = 'black';
			ctx.strokeStyle = 'black';

			//title
			var lTitleX, lTitleY;
			lTitleX = ctxLeft + 0.5 * ctxWidth; //does not depend on orientation
			if (orientation === 0) {//horizontal
				lTitleY = ctxTop - tickLength - fontSize;	
			} else {//vertical
				lTitleY = ctxTop + ctxHeight - fontSize;	
			}
			ctx.fillText(title, lTitleX - ctx.measureText(title).width * 0.5, lTitleY);

			//ticks and labels
			var lTickStartX, lTickEndX;
			var lTickStartY, lTickEndY;
			var dx = orientation === 0? ctxWidth / (numberOfLabels - 1) : ctxHeight / (numberOfLabels - 1);
			for (var i = 0; i < numberOfLabels; i++) {
				if (orientation === 0) {
					lTickStartX = ctxLeft + dx * i;
					lTickEndX = lTickStartX;
					if (labelOnLeft == 0) {
						lTickStartY = ctxTop;
						lTickEndY = lTickStartY - tickLength;	
					} else {
						lTickStartY = ctxTop + ctxHeight;
						lTickEndY = lTickStartY + tickLength;	
					}
				} else {//vertical
					lTickStartX = ctxLeft + ctxWidth;
					lTickEndX = lTickStartX + tickLength;
					lTickStartY = ctxTop + ctxHeight + i * dx;
					lTickEndY = lTickStartY;	
				}

				ctx.beginPath();       // Start a new path
				ctx.moveTo(lTickStartX, lTickStartY);    // Move the pen to (30, 50)
				ctx.lineTo(lTickEndX, lTickEndY);  // Draw a line to (150, 100)
				ctx.stroke();          // Render the path

				var value = minValue + i * (maxValue - minValue)/(numberOfLabels - 1);
				if (orientation !== 0) {
					value = maxValue - value;
				}
				var txt = value.toPrecision(4);
				if (orientation == 0) {
					var y = lTickEndY - 1;
					if (labelOnLeft != 0)
						y += fontSize;
					ctx.fillText(txt, lTickStartX - ctx.measureText(txt).width * 0.5, y);
				} else {
					var h = fontSize;
					ctx.fillText(txt, lTickEndX + 1, lTickEndY + h * 0.25);
				}
			}
		}
	};
	XHX.ColorBar.prototype.constructor = XHX.ColorBar;

})();
