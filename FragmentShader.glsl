//
// Angepasst f�r Core Profile
// ---------------------------------
//
// @author: Link Alexis Constantin, Andreas Klein
// @lecturer: Prof. Dr. Alfred Nischwitz
//
// (c)2010 Hochschule M�nchen, HM
//
// ---------------------------------
#version 400
smooth in vec2 texCoords;
in vec4 color;
out vec4 fragColor;

uniform samplerRect textureMap;

void main()
{
	fragColor = color * texture(textureMap, texCoords);
}