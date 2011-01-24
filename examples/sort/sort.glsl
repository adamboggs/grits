void main()
{
	gl_FrontColor = gl_Color;
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

	gl_Position[0] = 1; //gl_Vertex[0];
	gl_Position[1] = 1; //gl_Vertex[1];
	gl_Position[2] = 1; //gl_Vertex[2];

	vec4 projected = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_Position = projected;
	//gl_Position[3] = 100 + gl_Vertex[3];
	//gl_Position = gl_Vertex;
}
