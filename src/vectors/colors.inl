inline float3 color3_from_rgb_255(unsigned int r, unsigned int g, unsigned int b)
{
	return float3(r / 255.0f, g / 255.0f, b / 255.0f);
}

inline float3 color3_from_hex(unsigned int hex)
{
	MY_ENGINE_WARNING(hex <= 0x00FFFFFF, 
		"Creating float3 from hex with value over 0x00FFFFFF. "
		"You probably want to use float4::hex with alpha, or use smaller value. "
		"float3::hex only used 6 rightmost hex digits, or 24 least significant bits.")

	unsigned int r = 0xFF & (hex >> 16);				
	unsigned int g = 0xFF & (hex >> 8);				
	unsigned int b = 0xFF & hex;
	
	return color3_from_rgb_255(r,g,b);						
}