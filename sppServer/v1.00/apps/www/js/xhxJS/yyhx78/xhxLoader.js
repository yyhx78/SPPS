/**
 * @author mrdoob / http://mrdoob.com/
 * @author Alex Pletzer
 */

THREE.xhxLoader = function( aStudy ) {

	this.manager = THREE.DefaultLoadingManager;
	this.study = aStudy;
};

THREE.xhxLoader.prototype = {

	constructor: THREE.xhxLoader,

	load: function (url, onLoad, onProgress, onError ) {

		var scope = this;

		var sCmds = url.split('?');
		if (sCmds.length == 2 && sCmds[1].indexOf('cmd=') == 0) {

			var sCmd = sCmds[1].split('=');
			if (sCmd.length > 1) {
				var iCmdId = parseInt(sCmd[1]);
				if (iCmdId == 2) {//load study list
					var request = new XMLHttpRequest();
					request.onreadystatechange = function() {
						if (this.readyState == 4 && this.status == 200) {
							var studies = JSON.parse(this.responseText);

							onLoad(studies);
						}
					};
				
					request.open("POST", url, true);
					request.send();
				} else if (iCmdId == 1) {//load a study
					var request = new XMLHttpRequest();
					request.onreadystatechange = function() {
						if (this.readyState == 4 && this.status == 200) {
							var project = JSON.parse(this.responseText);

							onLoad(project);
						}
					};
				
					request.open("POST", url, true);
					request.send();					
				} else if (iCmdId == 3) {//load a study part
					var request = new XMLHttpRequest();
		            request.responseType = "arraybuffer";

					request.onreadystatechange = function() {
						if (this.readyState == 4 && this.status == 200) {
							var part = scope.parsePart(this.response);

							if (part !== undefined && part.fragments != undefined) {
					            var bbox = new THREE.Box3();
								for (iFrag = 0; iFrag < part.fragments.length; iFrag++) {
									if (iFrag == 0)
										bbox = part.fragments[iFrag].triGeo.boundingBox;
									else
									    bbox.union(part.fragments[iFrag].triGeo.boundingBox);
								}
								part.bbox = bbox;
							}

							onLoad(part);
						}
					};
				
					request.open("POST", url, true);
					request.send();					
				} else if (iCmdId == 4) { //colors (results)
					var request = new XMLHttpRequest();
		            request.responseType = "arraybuffer";

					request.onreadystatechange = function() {
						if (this.readyState == 4 && this.status == 200) {
							var result = scope.parseResult(this.response);

							onLoad(result);
						}
					};
				
					request.open("POST", url, true);
					request.send();	
				} else if (iCmdId == 5) { //cutting planes
					var request = new XMLHttpRequest();
		            request.responseType = "arraybuffer";

					request.onreadystatechange = function() {
						if (this.readyState == 4 && this.status == 200) {
							var result = scope.parseCut(this.response);

							onLoad(result);
						}
					};
				
					request.open("POST", url, true);
					request.send();					
				} else if (iCmdId == 6) { //iso-surfaces
					var request = new XMLHttpRequest();
		            request.responseType = "arraybuffer";

					request.onreadystatechange = function() {
						if (this.readyState == 4 && this.status == 200) {
							var result = scope.parseContours(this.response);

							onLoad(result);
						}
					};
				
					request.open("POST", url, true);
					request.send();										
				} else if (iCmdId == 8) { //mesh edges of a part (mesh component). 
					//Note: this is only for one part, there may be multiple parts.
					var request = new XMLHttpRequest();
		            request.responseType = "arraybuffer";

					request.onreadystatechange = function() {
						if (this.readyState == 4 && this.status == 200) {
							var result = scope.parseMeshEdges(this.response);
							onLoad(result);
						}
					};
				
					request.open("POST", url, true);
					request.send();	
				} else if (iCmdId == 10) { //query by server 
					//Note: this is only for one part, there may be multiple parts.
					var request = new XMLHttpRequest();
		            request.responseType = "arraybuffer";

					request.onreadystatechange = function() {
						if (this.readyState == 4 && this.status == 200) {
							onLoad(this.response);
						}
					};
				
					request.open("POST", url, true);
					request.send();	
				} else if (iCmdId == 11) { //query by server 
				//Note: this is only for one part, there may be multiple parts.
				var request = new XMLHttpRequest();
				request.responseType = "arraybuffer";

				request.onreadystatechange = function() {
					if (this.readyState == 4 && this.status == 200) {
						var result = scope.parseGlyphs(this.response);
						onLoad(result);
					}
				};
			
				request.open("POST", url, true);
				request.send();	
			}
		}

		} else {
			var loader = new THREE.XHRLoader( scope.manager );
			loader.setResponseType( 'arraybuffer' );
			loader.load( url, function( text ) {

				onLoad( scope.parse(url, text ) );

			}, onProgress, onError );
		}
	},

	parsePart: function ( resp ) {
		if (this.study === undefined)
		   return;
	    var iPosA = 0;
    	var iPartId = new Int32Array(resp, iPosA, 1);
    	iPosA += 4;
    	var iPart = iPartId[0];
    	var part = this.study.data.parts[iPart];

		part.type = 'mesh';
		part.fragments = [];//array of BufferGeometry
		var lFragments = [];
    	var iPos = this.readFragments(lFragments, resp, iPosA);
		if (lFragments.length > 0) {
			for (iFrag = 0; iFrag < lFragments.length; iFrag++) {
				var lFrag = lFragments[iFrag];
				var triGeo = new THREE.BufferGeometry();

				if (lFrag.vtxBuffer !== undefined)
					triGeo.addAttribute( 'position', new THREE.BufferAttribute( lFrag.vtxBuffer, 3 ) );
				if (lFrag.vtxNormals !== undefined)
					triGeo.addAttribute( 'normal', new THREE.BufferAttribute( lFrag.vtxNormals, 3, true ) );
				if (lFrag.triIndices !== undefined)
					triGeo.setIndex( new THREE.BufferAttribute( new Uint16Array( lFrag.triIndices ), 1 ) );
	
				triGeo.computeBoundingBox();

				part.fragments[iFrag] = {};
				part.fragments[iFrag].triGeo = triGeo;

				if (lFrag.featureEdgeIndices != undefined) {
					var feGeo = new THREE.BufferGeometry();
//					feGeo.addAttribute( 'position', new THREE.BufferAttribute( lFrag.vtxBuffer, 3 ) );
					feGeo.addAttribute( 'position', triGeo.getAttribute('position')); //share the same vertex buffer
					feGeo.setIndex( new THREE.BufferAttribute( new Uint16Array( lFrag.featureEdgeIndices ), 1 ) );

					part.fragments[iFrag].feGeo = feGeo;
				}
			}

		}

		return part;
	},

	parseResult: function ( resp ) {
		if (this.study === undefined)
		   return;
		var rlt = {};

		var iPos = 0;
		var nParts = new Uint32Array(resp, iPos, 1);
		iPos += 4;
		rlt.nParts = nParts[0];
		if (rlt.nParts != this.study.data.parts.length) {
			//something wrong
		}
		rlt.parts = [];
		if (rlt.nParts > 0)
		{
			var f2 = new Float32Array(resp, iPos, 2);
			iPos += 8;
			rlt.f2ScaleRange = [];
			rlt.f2ScaleRange[0] = f2[0];
			rlt.f2ScaleRange[1] = f2[1];

			for (iPart = 0; iPart < rlt.nParts; iPart++)
			{
				rlt.parts[iPart] = {};

				var iElementStatusApplied = new Int32Array(resp, iPos, 1);
				iPos += 4;

				this.study.data.parts[iPart].elementStatusApplied = iElementStatusApplied;//1: element status, 2: streamline, both are variable mesh data
				
				if (iElementStatusApplied == 2) {//2: streamlines
					var lFragments = [];
					var jPos = this.readFragments(lFragments, resp, iPos);
					if (lFragments.length > 0) {
						for (iFrag = 0; iFrag < lFragments.length; iFrag++) {
							var lFrag = lFragments[iFrag];
							var lineGeo = new THREE.BufferGeometry();
		
							if (lFrag.vtxBuffer !== undefined)
								lineGeo.addAttribute( 'position', new THREE.BufferAttribute( lFrag.vtxBuffer, 3 ) );
							if (lFrag.edgeIndices !== undefined)
								lineGeo.setIndex( new THREE.BufferAttribute( new Uint16Array( lFrag.edgeIndices ), 1 ) );
		
							if (lFrag.texCoords != undefined) {//color, if exists
								var ts = lFrag.texCoords;
								var nValues = ts.length;
								var uvs = new Float32Array(nValues * 2);//three.js requires 2d texture
								for (i = 0; i < nValues; i++) {
									uvs[2 * i] = (ts[i] - f2[0])/(f2[1] - f2[0]); //normalize
									uvs[2 * i + 1] = uvs[2 * i];//0.5;
								}
								lineGeo.addAttribute( 'uv', new THREE.BufferAttribute( uvs, 2, true ) );
								lineGeo.attributes.uv.needsUpdate = true;
							}
				
							lineGeo.computeBoundingBox();
		
							rlt.parts[iPart] = {};
							rlt.parts[iPart].lineGeo = lineGeo;
						}
					}

					continue; //go to next part
				} 

				var nPieces = new Int32Array(resp, iPos, 1);
				iPos += 4;
				if (this.study.data.parts[iPart].fragments.length != nPieces[0]) {
					//something wrong
				}
				rlt.parts[iPart].nPieces = nPieces[0];
				for (iPiece = 0; iPiece < nPieces[0]; iPiece++)
				{
					var lFrag = this.study.data.parts[iPart].fragments[iPiece].triGeo;

					var nTs;
					if (iElementStatusApplied == 1) {//1: modified mesh, some elements may be hidden
						//update the mesh because some elements may have disappeared,
						//consistent with Server::cmdReadResult()

//						lFrag = new THREE.BufferGeometry();
//						this.study.data.parts[iPart].fragments[iPiece].triGeo = lFrag; //reset
						//reset all atributes
						lFrag.removeAttribute( 'position');
						lFrag.removeAttribute( 'normal');
						lFrag.removeAttribute( 'uv');
						lFrag.removeAttribute( 'warp');

						//position
						nTs = new Int32Array(resp, iPos, 1);
						iPos += 4;
						if (nTs[0] > 0)
						{//vertexes
							var xyzs = new Float32Array(resp, iPos, nTs[0]);
							iPos += (4 * nTs[0]);
							lFrag.addAttribute( 'position', new THREE.BufferAttribute( xyzs, 3, false ) );
							lFrag.attributes.position.needsUpdate = true;
						}
						//normal
						nTs = new Int32Array(resp, iPos, 1);
						iPos += 4;
						if (nTs[0] > 0)
						{//vertexes
							var xyzs = new Float32Array(resp, iPos, nTs[0]);
							iPos += (4 * nTs[0]);
							lFrag.addAttribute( 'normal', new THREE.BufferAttribute( xyzs, 3, true ) );
							lFrag.attributes.normal.needsUpdate = true;
						}

						//triangle indices
						if ((iPos%4)!= 0)
							iPos += 2;
						var nIs = new Int32Array(resp, iPos, 1);
						iPos += 4;
						if (nIs[0] > 0)
						{
							if ((iPos%4)!= 0)
								iPos += 2;
							var triIndices = new Uint16Array(resp, iPos, nIs[0]);
							iPos += (2 * nIs[0]);
							if ((iPos%4)!= 0)
								iPos += 2;
							lFrag.setIndex( new THREE.BufferAttribute( new Uint16Array( triIndices ), 1 ) );
						}
					}
	
					var nTs = new Int32Array(resp, iPos, 1);
					iPos += 4;
					if (nTs[0] > 0)
					{//color
						var texCoords = new Float32Array(resp, iPos, nTs[0]);
						iPos += (4 * nTs[0]);
						var uvs = new Float32Array(nTs[0] * 2);//three.js requires 2d texture
						for (i = 0; i < texCoords.length; i++) {
							uvs[2 * i] = (texCoords[i] - f2[0])/(f2[1] - f2[0]); //normalize
							uvs[2 * i + 1] = uvs[2 * i];//0.5;
						}
						lFrag.addAttribute( 'uv', new THREE.BufferAttribute( uvs, 2, true ) );
						lFrag.attributes.uv.needsUpdate = true;
					}
					var nTs = new Int32Array(resp, iPos, 1);
					iPos += 4;
					if (nTs[0] > 0)
					{//displacement
						var warpXyz = new Float32Array(resp, iPos, nTs[0]);
						iPos += (4 * nTs[0]);
						
						lFrag.addAttribute( 'warp', new THREE.BufferAttribute( warpXyz, 3, false) );
						lFrag.attributes.warp.needsUpdate = true;
					}
				}
			}
		}
    
	    return rlt; //arrays have been included in the fragment geometry
	},

	parseCut: function(resp) {
		var rlt = {}; //return data: parts and clip values

		var parts = [];
		rlt.parts = parts;

		//start parsing
		var iPosA = 0;
		var nCuts = new Int32Array(resp, iPosA, 1);
		iPosA += 4;
		for (iCut = 0; iCut < nCuts[0]; iCut++)
		{
			var part = {};
			part.type = 'cut';
			rlt.parts[iCut] = part;
			
			part.fragments = [];//array of BufferGeometry
			var lFragments = [];
			var iPos = this.readFragments(lFragments, resp, iPosA);
			if (lFragments.length > 0) {
				for (iFrag = 0; iFrag < lFragments.length; iFrag++) {
					var lFrag = lFragments[iFrag];
					var triGeo = new THREE.BufferGeometry();

					if (lFrag.vtxBuffer !== undefined)
						triGeo.addAttribute( 'position', new THREE.BufferAttribute( lFrag.vtxBuffer, 3 ) );
					if (lFrag.triIndices !== undefined)
						triGeo.setIndex( new THREE.BufferAttribute( new Uint16Array( lFrag.triIndices ), 1 ) );

					if (lFrag.texCoords != undefined) {//color, if exists
						var f2 = this.study.data.result.f2ScaleRange;
						var ts = lFrag.texCoords;
						var nValues = ts.length;
						var uvs = new Float32Array(nValues * 2);//three.js requires 2d texture
						for (i = 0; i < nValues; i++) {
							uvs[2 * i] = (ts[i] - f2[0])/(f2[1] - f2[0]); //normalize
							uvs[2 * i + 1] = uvs[2 * i];//0.5;
						}
						triGeo.addAttribute( 'uv', new THREE.BufferAttribute( uvs, 2, true ) );
						triGeo.attributes.uv.needsUpdate = true;

					}
		
					triGeo.computeBoundingBox();

					part.fragments[iFrag] = {};
					part.fragments[iFrag].triGeo = triGeo;
				}
			}
			
			iPosA = iPos;
		}

		return rlt;
	},

	parseContours: function ( resp ) {
		var rlt = {}; //return data: parts and clip values

		var parts = [];
		rlt.parts = parts;

		//start parsing
		var iPosA = 0;
		var nVals = new Int32Array(resp, iPosA, 1);
		iPosA += 4;
		
		var clipVals = new Float32Array(resp, iPosA, nVals[0]);
		iPosA += (nVals[0] * 4);
		rlt.clipVals = clipVals;
		
		var nPDs = new Int32Array(resp, iPosA, 1);
		iPosA += 4;
		
		var iTotal = 0;
		for (iIso = 0; iIso < nPDs[0]; iIso++)
		{
			var part = {};
			part.type = 'iso';

			rlt.parts[iIso] = part;
			part.clipValue = clipVals[iIso];

			part.fragments = [];//array of BufferGeometry
			var lFragments = [];
			var iPos = this.readFragments(lFragments, resp, iPosA);
			if (lFragments.length > 0) {
				for (iFrag = 0; iFrag < lFragments.length; iFrag++) {
					var lFrag = lFragments[iFrag];
					var triGeo = new THREE.BufferGeometry();

					if (lFrag.vtxBuffer !== undefined)
						triGeo.addAttribute( 'position', new THREE.BufferAttribute( lFrag.vtxBuffer, 3 ) );
					if (lFrag.vtxNormals !== undefined)
						triGeo.addAttribute( 'normal', new THREE.BufferAttribute( lFrag.vtxNormals, 3, true ) );
					if (lFrag.triIndices !== undefined)
						triGeo.setIndex( new THREE.BufferAttribute( new Uint16Array( lFrag.triIndices ), 1 ) );
		
					triGeo.computeBoundingBox();

					part.fragments[iFrag] = {};
					part.fragments[iFrag].triGeo = triGeo;
				}
			}
			
			iTotal++;
			
			iPosA = iPos;
		}

		return rlt;
	},

	readFragments: function(fragments, resp, iPos)
	{
    	var nPieces = new Int32Array(resp, iPos, 1);
    	iPos += 4;
		for (iPiece = 0; iPiece < nPieces[0]; iPiece++)
		{
				fragments[iPiece] = {};

			var lFrag = fragments[iPiece];
			
			//vertex buffer
			var nFs = new Int32Array(resp, iPos, 1);
			iPos += 4;
			if (nFs[0] > 0)
			{
				lFrag.vtxBuffer = new Float32Array(resp, iPos, nFs[0]);
				iPos += (4 * nFs[0]);
			}

			//texture coordinates
			nFs = new Int32Array(resp, iPos, 1);
			iPos += 4;
			if (nFs[0] > 0)
			{
				lFrag.texCoords = new Float32Array(resp, iPos, nFs[0]);
				iPos += (4 * nFs[0]);
			}
			
			//vertex normal
			nFs = new Int32Array(resp, iPos, 1);
			iPos += 4;
			if (nFs[0] > 0)
			{
				lFrag.vtxNormals = new Float32Array(resp, iPos, nFs[0]);
				iPos += (4 * nFs[0]);	
			}

			//edge indices
			var nIs = new Int32Array(resp, iPos, 1);
			iPos += 4;
			if (nIs[0] > 0)
			{
				lFrag.edgeIndices = new Uint16Array(resp, iPos, nIs[0]);
				iPos += (2 * nIs[0]);
			}
			
			//feature edge indices
			if ((iPos%4)!= 0)
				iPos += 2;
			nIs = new Int32Array(resp, iPos, 1);
			iPos += 4;
			if (nIs[0] > 0)
			{
				if ((iPos%4)!= 0)
				iPos += 2;
				lFrag.featureEdgeIndices = new Uint16Array(resp, iPos, nIs[0]);
				iPos += (2 * nIs[0]);
			}
			
			//triangle indices
			if ((iPos%4)!= 0)
				iPos += 2;
			nIs = new Int32Array(resp, iPos, 1);
			iPos += 4;
			if (nIs[0] > 0)
			{
				if ((iPos%4)!= 0)
				iPos += 2;
				lFrag.triIndices = new Uint16Array(resp, iPos, nIs[0]);
				iPos += (2 * nIs[0]);
			}
			if ((iPos%4)!= 0)
				iPos += 2;
		}

		return iPos;
	},

	parseMeshEdges: function(resp) {
		if (this.study === undefined)
			return;
			
		var rlt = {};

		var iPosA = 0;
		var iPartId = new Int32Array(resp, iPosA, 1);
		iPosA += 4;

		if (iPartId[0] >= 0 && iPartId[0] < this.study.data.parts.length)
		{
			var iPart = iPartId[0];
			var lPart = this.study.data.parts[iPart];   

			rlt.iPart = iPart;
			rlt.part = lPart; //mesh edges will be included in part.fragments.meshEdgeGeo. 

			var lFragments = [];
			//first section is for triangles
			var iPos = this.readFragments(lFragments, resp, iPosA);
			iPosA = iPos;
			//second section is for mesh edges
			var iPos = this.readFragments(lFragments, resp, iPosA);
			iPosA = iPos;
			//3rd section is for feature edges
			var iPos = this.readFragments(lFragments, resp, iPosA);
			if (lFragments.length > 0) {
				for (iFrag = 0; iFrag < lFragments.length; iFrag++) {
					var triGeo = lPart.fragments[iFrag].triGeo;
					if (triGeo === undefined)
						continue;

					var lFrag = lFragments[iFrag];
					if (lFrag.edgeIndices != undefined)
					{
						var mshEdgeGeo = new THREE.BufferGeometry();

						//position array is shared
						mshEdgeGeo.addAttribute('position', new THREE.BufferAttribute( lFrag.vtxBuffer, 3 ) );
						mshEdgeGeo.setIndex( new THREE.BufferAttribute( new Uint16Array( lFrag.edgeIndices ), 1 ) );
			
						lPart.fragments[iFrag].meshEdgeGeo = mshEdgeGeo;	
					}

					if (lFrag.featureEdgeIndices != undefined)
					{
						var featureEdgeGeo = new THREE.BufferGeometry();

						//position array is shared
						featureEdgeGeo.addAttribute('position', new THREE.BufferAttribute( lFrag.vtxBuffer, 3 ) );
						featureEdgeGeo.setIndex( new THREE.BufferAttribute( new Uint16Array( lFrag.featureEdgeIndices ), 1 ) );
			
						lPart.fragments[iFrag].feGeo = featureEdgeGeo;	
					}
				}
			}
		}

		return rlt;
	},

	parseGlyphs: function(resp) {
		if (this.study === undefined)
			return;
			
		var rlt = {};

	//locations 
		var iPos = 0;
		var lPtCount = new Int32Array(resp, iPos, 1);
		iPos += 4;

		if (lPtCount[0] > 0)
		{
			rlt.nPts = lPtCount[0];
			rlt.positions = new Float32Array(resp, iPos, rlt.nPts * 3);
			iPos += (4 * rlt.nPts * 3);
		}

	//result (color)
		//nPts again
		var lPtCount = new Int32Array(resp, iPos, 1);
		iPos += 4;
		if (lPtCount[0] > 0)
		{
			rlt.nPts = lPtCount[0];//should be the same

			var lCmptCount = new Int32Array(resp, iPos, 1);
			iPos += 4;
			rlt.nCmpts = lCmptCount[0];

			rlt.rltValues = new Float32Array(resp, iPos, rlt.nPts * rlt.nCmpts);
			iPos += (4 * rlt.nPts * rlt.nCmpts);
		}

		return rlt;
	}
};
