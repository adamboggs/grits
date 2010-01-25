void main()
{
	gl_Position =
		gl_ModelViewProjectionMatrix *
		( gl_Vertex + 0.5 );
	gl_Position[3] = 1;
}
