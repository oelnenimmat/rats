#ifndef RANDOM_GLSL_INCLUDED
#define RANDOM_GLSL_INCLUDED

/*
    static.frag
    by Spatial
    05 July 2013

    https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl

    other resources:
    http://lukas-polok.cz/tutorial_sphere.htm
    https://gist.github.com/superwills/569e1be1242e92051700994d53cbfb8f
*/

// #version 330 core

// uniform float time;
// out vec4 fragment;

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

// Compound versions of the hashing algorithm I whipped together.
uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}



// Pseudo-random value in half-open range [0:1].
float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
float random( vec2  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec4  v ) { return floatConstruct(hash(floatBitsToUint(v))); }

vec3 random_vec3_in_unit_cube( vec3 v )
{
    return vec3(
        floatConstruct(hash(floatBitsToUint(v.yz))),
        floatConstruct(hash(floatBitsToUint(v.zx))),
        floatConstruct(hash(floatBitsToUint(v.xy)))
    );
}

vec3 random_direction(vec3 v)
{
    // https://stackoverflow.com/questions/38112526/why-do-people-use-sqrtdotdistancevector-distancevector-over-opengls-distan
    // this returns somehting from normalized cube, which is problematic, but that is best i have right now
    // since i do not know how to get actual random in gpu
    return normalize(random_vec3_in_unit_cube(v) - vec3(0.5, 0.5, 0.5));
}




/*
void main()
{
    vec3  inputs = vec3( gl_FragCoord.xy, time ); // Spatial and temporal inputs
    float rand   = random( inputs );              // Random per-pixel value
    vec3  luma   = vec3( rand );                  // Expand to RGB

    fragment = vec4( luma, 1.0 );
}
*/

#endif // RANDOM_GLSL_INCLUDED