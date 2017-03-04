#version 150 core

# define N 8

uniform sampler2DShadow uShadowMap0;
uniform sampler2D uProjectorTex0;
uniform sampler2DShadow uShadowMap1;
uniform sampler2D uProjectorTex1;
uniform sampler2DShadow uShadowMap2;
uniform sampler2D uProjectorTex2;
uniform sampler2DShadow uShadowMap3;
uniform sampler2D uProjectorTex3;
uniform sampler2DShadow uShadowMap4;
uniform sampler2D uProjectorTex4;
uniform sampler2DShadow uShadowMap5;
uniform sampler2D uProjectorTex5;
uniform sampler2DShadow uShadowMap6;
uniform sampler2D uProjectorTex6;
uniform sampler2DShadow uShadowMap7;
uniform sampler2D uProjectorTex7;
uniform vec3 uLightPos[N];

in vec4 vColor;
in vec4 vPosition;
in vec3 vNormal;
in vec4 vShadowCoord[N];

out vec4 Color;

void main( void )
{
	vec3 Normal			= normalize( vNormal );
	vec3 Ambient		= vec3( 0.1 );
	float Shadow		= 1.0;
    vec4 projTexColor = vec4( 0.0, 0.0, 1.0, 0.0);
    
    vec4 tColor = vec4( 0.0, 0.0, 0.0, 0.0);
    tColor.rgb = ( Ambient ) * vColor.rgb;
    
    // projector 0
    vec3 LightVec0		= normalize(uLightPos[0] - vPosition.xyz);
    float NdotL0		= max(dot(vNormal, LightVec0), 0.0);
    vec3 Diffuse0		= vec3(NdotL0);
    vec4 ShadowCoord0	= vShadowCoord[0] / vShadowCoord[0].w;
	if(ShadowCoord0.z > 0.0){
		Shadow = textureProj(uShadowMap0, ShadowCoord0, -0.00005);
        projTexColor = textureProj(uProjectorTex0, ShadowCoord0);
	}
    if(ShadowCoord0.x > 0 && ShadowCoord0.x < 1 && ShadowCoord0.y > 0 && ShadowCoord0.y < 1){
        tColor.rgb = (Diffuse0 * Shadow) * projTexColor.rgb + tColor.rgb;
    }
    
    // projector 1
    vec3 LightVec1		= normalize(uLightPos[1] - vPosition.xyz);
    float NdotL1		= max(dot(vNormal, LightVec1), 0.0);
    vec3 Diffuse1		= vec3(NdotL1);
    vec4 ShadowCoord1	= vShadowCoord[1] / vShadowCoord[1].w;
    if(ShadowCoord1.z > 0.0){
        Shadow = textureProj(uShadowMap1, ShadowCoord1, -0.00005);
        projTexColor = textureProj(uProjectorTex1, ShadowCoord1);
    }
    if(ShadowCoord1.x > 0 && ShadowCoord1.x < 1 && ShadowCoord1.y > 0 && ShadowCoord1.y < 1){
        tColor.rgb = (Diffuse1 * Shadow) * projTexColor.rgb + tColor.rgb;
    }
    
    // projector 2
    vec3 LightVec2		= normalize(uLightPos[2] - vPosition.xyz);
    float NdotL2		= max(dot(vNormal, LightVec2), 0.0);
    vec3 Diffuse2		= vec3(NdotL2);
    vec4 ShadowCoord2	= vShadowCoord[2] / vShadowCoord[2].w;
    if(ShadowCoord2.z > 0.0){
        Shadow = textureProj(uShadowMap2, ShadowCoord2, -0.00005);
        projTexColor = textureProj(uProjectorTex2, ShadowCoord2);
    }
    if(ShadowCoord2.x > 0 && ShadowCoord2.x < 1 && ShadowCoord2.y > 0 && ShadowCoord2.y < 1){
        tColor.rgb = (Diffuse2 * Shadow) * projTexColor.rgb + tColor.rgb;
    }
    
    // projector 3
    vec3 LightVec3		= normalize(uLightPos[3] - vPosition.xyz);
    float NdotL3		= max(dot(vNormal, LightVec3), 0.0);
    vec3 Diffuse3		= vec3(NdotL3);
    vec4 ShadowCoord3	= vShadowCoord[3] / vShadowCoord[3].w;
    if(ShadowCoord3.z > 0.0){
        Shadow = textureProj(uShadowMap3, ShadowCoord3, -0.00005);
        projTexColor = textureProj(uProjectorTex3, ShadowCoord3);
    }
    if(ShadowCoord3.x > 0 && ShadowCoord3.x < 1 && ShadowCoord3.y > 0 && ShadowCoord3.y < 1){
        tColor.rgb = (Diffuse3 * Shadow) * projTexColor.rgb + tColor.rgb;
    }
    
    // projector 4
    vec3 LightVec4		= normalize(uLightPos[4] - vPosition.xyz);
    float NdotL4		= max(dot(vNormal, LightVec4), 0.0);
    vec3 Diffuse4		= vec3(NdotL4);
    vec4 ShadowCoord4	= vShadowCoord[4] / vShadowCoord[4].w;
    if(ShadowCoord4.z > 0.0){
        Shadow = textureProj(uShadowMap4, ShadowCoord4, -0.00005);
        projTexColor = textureProj(uProjectorTex4, ShadowCoord4);
    }
    if(ShadowCoord4.x > 0 && ShadowCoord4.x < 1 && ShadowCoord4.y > 0 && ShadowCoord4.y < 1){
        tColor.rgb = (Diffuse4 * Shadow) * projTexColor.rgb + tColor.rgb;
    }
    
    // projector 5
    vec3 LightVec5		= normalize(uLightPos[5] - vPosition.xyz);
    float NdotL5		= max(dot(vNormal, LightVec5), 0.0);
    vec3 Diffuse5		= vec3(NdotL5);
    vec4 ShadowCoord5	= vShadowCoord[5] / vShadowCoord[5].w;
    if(ShadowCoord5.z > 0.0){
        Shadow = textureProj(uShadowMap5, ShadowCoord5, -0.00005);
        projTexColor = textureProj(uProjectorTex5, ShadowCoord5);
    }
    if(ShadowCoord5.x > 0 && ShadowCoord5.x < 1 && ShadowCoord5.y > 0 && ShadowCoord5.y < 1){
        tColor.rgb = (Diffuse5 * Shadow) * projTexColor.rgb + tColor.rgb;
    }
    
    // projector 6
    vec3 LightVec6		= normalize(uLightPos[6] - vPosition.xyz);
    float NdotL6		= max(dot(vNormal, LightVec6), 0.0);
    vec3 Diffuse6		= vec3(NdotL6);
    vec4 ShadowCoord6	= vShadowCoord[6] / vShadowCoord[6].w;
    if(ShadowCoord6.z > 0.0){
        Shadow = textureProj(uShadowMap6, ShadowCoord6, -0.00005);
        projTexColor = textureProj(uProjectorTex6, ShadowCoord6);
    }
    if(ShadowCoord6.x > 0 && ShadowCoord6.x < 1 && ShadowCoord6.y > 0 && ShadowCoord6.y < 1){
        tColor.rgb = (Diffuse6 * Shadow) * projTexColor.rgb + tColor.rgb;
    }
    
    // projector 7
    vec3 LightVec7		= normalize(uLightPos[7] - vPosition.xyz);
    float NdotL7		= max(dot(vNormal, LightVec7), 0.0);
    vec3 Diffuse7		= vec3(NdotL7);
    vec4 ShadowCoord7	= vShadowCoord[7] / vShadowCoord[7].w;
    if(ShadowCoord7.z > 0.0){
        Shadow = textureProj(uShadowMap7, ShadowCoord7, -0.00005);
        projTexColor = textureProj(uProjectorTex7, ShadowCoord7);
    }
    if(ShadowCoord7.x > 0 && ShadowCoord7.x < 1 && ShadowCoord7.y > 0 && ShadowCoord7.y < 1) {
        tColor.rgb = (Diffuse7* Shadow) * projTexColor.rgb + tColor.rgb;
    }
    
    Color.rgb = tColor.rgb;
    Color.a	= 1.0;
}