static const float PI = 3.14159265359;

float3 GetDirectionFromCubemapTexel(uint3 threadID, int resolution)
{
    float2 uv = (float2(threadID.xy) + 0.5f) / resolution;
    
    float u = uv.x * 2.0f - 1.0f;
    float v = uv.y * 2.0f - 1.0f;
    
    v = -v;
    
    float3 dir = float3(0.0f, 0.0f, 0.0f);
    
    switch (threadID.z)
    {
        case 0:
            dir = float3(1.0f, v, -u);
            break; // +X (오른쪽)
        case 1:
            dir = float3(-1.0f, v, u);
            break; // -X (왼쪽)
        case 2:
            dir = float3(u, 1.0f, -v);
            break; // +Y (위)
        case 3:
            dir = float3(u, -1.0f, v);
            break; // -Y (아래)
        case 4:
            dir = float3(u, v, 1.0f);
            break; // +Z (앞)
        case 5:
            dir = float3(-u, v, -1.0f);
            break; // -Z (뒤)
    }
    return normalize(dir);
}

// https://github.com/tuxalin/vulkanri/blob/master/examples/pbr_ibl/shaders/importanceSampleGGX.glsl
// Van Der Corpus sequence
// @see http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
float VdcSequence(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// Hammersley sequence
// @see http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
float2 HammersleySequence(uint i, uint N)
{
    return float2(float(i) / float(N), VdcSequence(i));
}

// GGX NDF via importance sampling
float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
	
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (alpha2 - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	
    // from spherical coordinates to cartesian coordinates
    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
	
    // from tangent-space vector to world-space sample vector
    float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangent = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);
	
    float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

// Schlick-GGX 단일 방향 차폐율
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float a = roughness;
    float k = (a * a) / 2.0; // IBL용 k 공식!
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}
// Smith's method: 두 방향(V, L)의 차폐율을 곱함
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    
    float ggx2 = GeometrySchlickGGX(NdotV, roughness); // Masking
    float ggx1 = GeometrySchlickGGX(NdotL, roughness); // Shadowing
    return ggx1 * ggx2;
}