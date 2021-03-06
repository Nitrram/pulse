#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec2 vertexPosition_modelspace;
//layout(location = 1) in vec2 vertexUV;

// Output data ; will be interpolated for each fragment.
out vec2 UV;
//out vec2 pixelPos;

void main(){

	// Output position of the vertex, in clip space : MVP * position
	gl_Position = vec4(vertexPosition_modelspace, 0 , 1);// + vec4(-1.0, 0, 0, 0);
	//pixelPos = gl_Position.xy;0

	//gl_Position += vec4(0, -0.9,0,0);
	/* vec4 projectedpos = mvpmatrix * vec4(in_Position, 1.0); */
	/* FinalPos = vec2(projectedpos.xy) / projectedpos.w; */
	/* FinalPos = (FinalPos * 0.5 + 0.5) * vec2(Viewportwidth,Viewportheight); */
	/* gl_Position = FinalPos; */

	// UV of the vertex. No special space for this one.
	UV = vertexPosition_modelspace * 0.5 + 0.5;
}
