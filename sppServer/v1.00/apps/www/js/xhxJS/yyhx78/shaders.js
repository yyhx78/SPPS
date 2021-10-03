var xhxRainbowShader = {

	uniforms: {},

	vertexShader: [
		"uniform int uCutting;",
		"uniform int uHasResult;",

        "varying vec4 vWorldSpaceNormal;",
		"varying vec3 vWorldSpacePos;",
		"varying vec2 vUV;",
		"varying vec3 vLighting;",

		"attribute vec3 warp;",

		"void main() {",
			"vUV = uv;",
			"vWorldSpacePos = position;",
			
			"if (uCutting != 1)",
			    "vWorldSpaceNormal = modelViewMatrix  * vec4(normal, 0.0);",

			"gl_Position = projectionMatrix * modelViewMatrix * vec4(vWorldSpacePos,1.0);",

			"vec3 ambientLight = vec3(0.3, 0.3, 0.3);",
			"vec3 directionalLightColor = vec3(1, 1, 1);",
			"vec3 directionalVector = normalize(vec3(0.85, 0.8, 0.75));",
	  
			"vec3 transformedNormal = normalMatrix * position;",
	  
			"float directional = dot(transformedNormal.xyz, directionalVector);",
			"if (directional < 0.0) directional = - directional;",
			"vLighting = ambientLight + (directionalLightColor * directional);",
		"}"
	].join( "\n" ),

	fragmentShader: [
		"uniform int uCutting;",
		"uniform int uSingleValueClipping;",
		"uniform float uClipValue;",
		"uniform vec4 uCuttingPlane;",
		"uniform sampler2D texture1;",
		"uniform int uHasResult;",

		"varying vec4 vWorldSpaceNormal;",
		"varying vec3 vWorldSpacePos;",
		"varying vec2 vUV;",
		"varying vec3 vLighting;",

		"void main()",
		"{",
			"if (uHasResult == 1 && uSingleValueClipping == 1 && uClipValue < vUV.x) {",
				"discard;",
			"} else if (uCutting == 1 && dot(uCuttingPlane.xyz, vWorldSpacePos) > uCuttingPlane[3]) {",
				"discard;",
			"} else",
			"{",
				"float diff = 1.0;",
				"if (uCutting == 1)",
					"diff = dot(vec3(0.3,0.3,1), uCuttingPlane.xyz);",
				"else",
					"diff = dot(vec3(0.3,0.3,1), vWorldSpaceNormal.xyz);",
				"if (diff < 0.0)",
					"diff = -diff;",

				"vec4 color;", 
				"if (uHasResult == 1) {",					
					"color = texture2D(texture1, vUV);", 
				"} else {",
					"color = vec4(0.1, 0.9, 0.5, 1.0);",
				"}",
				
				"gl_FragColor = vec4(max(color.rgb * diff, color.rgb * 0.8), 1);",
			"}",
		"}"
	].join( "\n" ),

	vertexShader_Iso: [
        "varying vec4 vWorldSpaceNormal;",

		"void main() {",
			"gl_Position = projectionMatrix * modelViewMatrix * vec4(position,1.0);",

		    "vWorldSpaceNormal = modelViewMatrix  * vec4(normal, 0.0);",
		"}"
	].join( "\n" ),

	fragmentShader_Iso: [
		"uniform sampler2D texture1;",
		"uniform float clipValue;",

		"varying vec4 vWorldSpaceNormal;",

		"void main()",
		"{",
			"float diff = 1.0;",
			"diff = dot(vec3(0.3,0.3,1), vWorldSpaceNormal.xyz);",
	        "if (diff < 0.0)",
    	        "diff = -diff;",
			
			"vec4 color = texture2D(texture1, vec2(clipValue, clipValue));", 
			
            "gl_FragColor = vec4(max(color.rgb * diff, color.rgb * 0.8), 1);",

		"}"
	].join( "\n" ),

	vertexShader_Cut: [
		"uniform int uHasResult;",

		"varying vec2 vUV;",
		
		"void main() {",
			"if (uHasResult == 1)", 
				"vUV = uv;",

			"gl_Position = projectionMatrix * modelViewMatrix * vec4(position,1.0);",
		"}"
	].join( "\n" ),

	fragmentShader_Cut: [
		"uniform sampler2D texture1;",
		"uniform vec3 uPlaneNormal;",
		"uniform int uHasResult;",

		"varying vec2 vUV;",

		"void main()",
		"{",
			"float diff = 1.0;",
			"diff = dot(vec3(0.3,0.3,1), uPlaneNormal);",
	        "if (diff < 0.0)",
    	        "diff = -diff;",
			
			"vec4 color;",
			"if (uHasResult == 1)",
				"color = texture2D(texture1, vUV);", 
			"else",
				"color = vec4(0.1, 0.9, 0.5, 1.0);", 
			
            "gl_FragColor = vec4(max(color.rgb * diff, color.rgb * 0.8), 1);",

		"}"
	].join( "\n" ),

	vertexShader_Edges: [
		"uniform int uHasResult;",
		"varying vec2 vUV;",
		"varying vec3 vWorldSpacePos;",
		"void main() {",
			"if (uHasResult == 1)", 
				"vUV = uv;",
			"vWorldSpacePos = position;",
			"gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );",
		"}"
	].join( "\n" ),

	fragmentShader_Edges: [
		"uniform sampler2D texture1;",
		"uniform int uHasResult;",
		"varying vec2 vUV;",
		"uniform int uCutting;",
		"uniform vec4 uCuttingPlane;",
		"uniform vec3 color;",
		"uniform float opacity;",
		"varying vec3 vWorldSpacePos;",
		"void main() {",
			"if (uCutting == 1 && dot(uCuttingPlane.xyz, vWorldSpacePos) > uCuttingPlane[3]) {",
				"discard;",
			"} else {",
				"if (uHasResult == 1)",
					"gl_FragColor = texture2D(texture1, vUV);",
				"else",
					"gl_FragColor = vec4( color, opacity );",
			"}",
		"}"
	].join( "\n" ),

	vertexShader_Instanced: [
		"precision highp float;",
	
		"uniform mat4 modelViewMatrix;",
		"uniform mat4 projectionMatrix;",
		"uniform float uGlobalScaleFactor;",

		"attribute float t;",
		"attribute float visibility;",
		"attribute vec3 position;",

		"varying vec2 vUv;",
		"varying float vVisibility;",

		"attribute vec4 aInstanceMatrix0;",
		"attribute vec4 aInstanceMatrix1;",
		"attribute vec4 aInstanceMatrix2;",
		"attribute vec4 aInstanceMatrix3;",

		"void main() {",
			"vVisibility = visibility;",
			"mat4 aInstanceMatrix = mat4(",
				"aInstanceMatrix0,",
				"aInstanceMatrix1,",
				"aInstanceMatrix2,",
				"aInstanceMatrix3",
			 ");",

			 "vec3 sPos = position * uGlobalScaleFactor;",

			"vec3 vPosition = (aInstanceMatrix * vec4( sPos , 1. )).xyz;",

			"vUv = vec2(t, t);",

			"gl_Position = projectionMatrix * modelViewMatrix * vec4( vPosition, 1.0 );",

		"}"
	].join( "\n" ),

	fragmentShader_Instanced: [
		"precision highp float;",
	
		"uniform sampler2D map;",

		"varying vec2 vUv;",
		"varying float vVisibility;",

		"void main() {",
			"if (vVisibility < 0.01)",
				"discard;",
			"else",
				"gl_FragColor = texture2D( map, vUv );",
		"}"
	].join( "\n" )
};
