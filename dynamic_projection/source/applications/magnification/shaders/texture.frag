#version 330 core
in vec2 UV;
out vec3 color;

//layout(origin_upper_left) in vec4 gl_FragCoord;

uniform sampler2D texture_sampler;
uniform vec2 mouse_pos;
uniform vec2 window_size;
uniform bool magnification;
uniform float magnification_factor;
uniform vec2 cursors[16];
uniform int num_cursors;

varying vec3 pos;

// measured in ???.
const float boundary = 0.125f;

// constants
const float EPSILON = 0.0001f;
const float PI = 3.14159265358979323846264;

// Change these to uniform variables
const float linear_magnification = 4;
const float outer_radius = 0.2;
const float inner_radius = 0.1;

struct WeightedUV{
    float dist;
    vec2 UV;
};


bool approx_zero( float in_val ){
    return EPSILON * -1.0f < in_val && in_val < EPSILON;
}

bool in_range( vec2 input_vec, float max_val ){
	float x_dist = abs(input_vec.x - mouse_pos.x);
	float y_dist = abs(input_vec.y - mouse_pos.y);
	return x_dist < max_val && y_dist < max_val;
}

vec2 linear_transform( vec2 input_vec, int cursor ){

	// Scales this way due to modifying UV.
	float scale = 1.0f / linear_magnification;
	// find distance between input UV and normalized mouse position
	float dist = distance( input_vec, cursors[cursor] );

	// find direction from mouse position to input UV.
	vec2 dir = normalize( input_vec - cursors[cursor] );

	// scale the distance to the new distance
	dist *= scale;

	// new UV coordinate is found by adding the mouse position to the
	// direction vector times the scaling factor.
	vec2 result = cursors[cursor] + dir * dist;

	return result;
}

vec2 polar_to_cartesian( vec2 input_vec, int cursor ){
    // origin = mouse pos 
    // r = distance from input_vec to mouse_pos
    // theta = angle between origin and point
    float x_dist = mouse_pos.x - input_vec.x;
    float y_dist = mouse_pos.y - input_vec.y;

    float r = sqrt( x_dist * x_dist + y_dist * y_dist );
    float theta;
    if ( approx_zero( x_dist ) ){
        if( y_dist > 0.0000f ){
            theta = PI / 2.0f;
        }
        else{
            theta = 3.0f * PI / 2.0f;
        }
    }
    theta = atan(y_dist / x_dist);
    return vec2( r, theta );
}

vec2 cartesian_to_polar( vec2 input_vec, int cursor ){
    float r = input_vec.x;
    float theta = input_vec.y;

    float x_offset = r * cos(theta);
    float y_offset = r * sin(theta);
    
    return vec2( mouse_pos.x + x_offset, mouse_pos.y + y_offset);
}

float max_distance( vec2 input_vec, int cursor ){
    float x_dist = mouse_pos.x - input_vec.x;
    float y_dist = mouse_pos.y - input_vec.y;

    if( approx_zero( y_dist )){
        if( x_dist > 0.0f){
            return 1.0f - mouse_pos.x;
        }
        else{
            return mouse_pos.x;
        }
    }
    else if( approx_zero( x_dist )){
        if( y_dist > 0.0f){
            return 1.0f - mouse_pos.y;
        }
        else{
            return mouse_pos.y;
        }
    }
    else{
        float m = x_dist / y_dist;
        float b = mouse_pos.y - mouse_pos.x * m;

        if( x_dist < 0.0f ){
            float new_y = b;
            if( y_dist < 0.0f){
                float new_x = -1.0f * b / m;

                if( new_x < 0.0f )
                    return distance( vec2( 0.0f, new_y ), mouse_pos );
                else
                    return distance( vec2( new_x, 0.0f ), mouse_pos );
                
            }
            else{
                float new_x = (1.0f - b )/ m;

                if( new_x < 0.0f )
                    return distance( vec2( 0.0f, new_y ), mouse_pos );
                else
                    return distance( vec2( new_x, 1.0f ), mouse_pos );
            }
        }
        else{
            float new_y = m + b;
            if( y_dist < 0.0f ){
                float new_x = -1.0f * b / m;

                if( new_x > 1.0f )
                    return distance( vec2( 1.0f, new_y ), mouse_pos );
                else
                    return distance( vec2( new_x, 0.0f ), mouse_pos );
            }
            else{
                float new_x = (1.0f - b ) /m;
                
                if( new_x > 1.0f ) 
                    return distance( vec2( 1.0f, new_y ), mouse_pos );
                else
                    return distance( vec2( new_x, 1.0f ), mouse_pos );
            }
        }
    }
}

vec2 logistic_regression( vec2 lower, vec2 upper ){
    float a = 0.0;
    float b = 0.0;

    float numerator = (upper.y + 0.001) - (lower.y - 0.001);
    float lhs = upper.y - lower.y + 0.001;

    a = ( numerator - lhs ) / lhs;
    b = log( (numerator/0.001 + 1)/ a ) / (lower.x - upper.x);

    return vec2(a, b);
}

float logistic_function( float input, vec2 lower, vec2 upper ){
    vec2 logistic_vars = logistic_regression( lower, upper);
    float numerator = ((upper.y + 0.001) - (lower.y - 0.001));
    float denominator = 1 + logistic_vars.x * exp( logistic_vars.y * ( input - upper.x ));
    return lower.y + 0.001 + numerator / denominator; 
}

vec2 fisheye_transform( vec2 input_vec, int cursor ){
    float x_dist = input_vec.x - cursors[cursor].x;
    float y_dist = input_vec.y - cursors[cursor].y;

    //float dist = distance(input_vec, cursors[cursor]);
    float dist = sqrt(x_dist * x_dist + y_dist * y_dist);

    vec2 outer = vec2( outer_radius, 1 );
    vec2 inner = vec2( inner_radius, linear_magnification );

    float slope = (inner.y - outer.y) / (inner.x - outer.x);
    float b = inner.y - slope * inner.x;

    float magnification_level = slope * dist + b;

    magnification_level = logistic_function( dist, outer, inner);


    float computed_radius = dist * magnification_level;


    if( dist > 0.0 ){

        float radius = computed_radius;
        float power = 10.0f;
        float power_exp = exp(power);
        //float new_dist = power_exp /(power_exp - 1.0f) * radius * ( 1.0f - exp( -dist / radius * power ) );

        float new_dist = exp(power) /(exp(power) - 1.0f) * radius /* * (1.0f - exp( -dist / radius * power ))*/;
        float new_x = cursors[cursor].x + x_dist * ( dist / new_dist /* * 3.0f/4.0f + 1.0f/4.0f */ );
        float new_y = cursors[cursor].y + y_dist * ( dist / new_dist /* * 3.0f/4.0f + 1.0f/4.0f */ );

        return vec2( new_x, new_y );
    }
    else{
        return input_vec;
    }
}

vec3 get_color(){
    int count = 0;
    WeightedUV results[16];
    float colorize = 0.0f;
    vec2 sum = vec2(0.0f, 0.0f);
    bool linear = false;
    bool fisheye = false;

    for( int i = 0; i < num_cursors; i++ ){
        if( magnification ){
            float dist = distance( pos.xy, cursors[i] );
            if( dist <= inner_radius ) {
                vec2 new_UV = linear_transform( UV, i );
                
                WeightedUV w_uv;
                w_uv.dist = outer_radius - dist;
                if( w_uv.dist == 0 ){
                    //return vec3( 1, 1, 1);
                }
                w_uv.UV = new_UV;
                
                results[count] = w_uv;
                count++;
                linear = true;
            }
            else if( dist <= outer_radius ){
                vec2 new_UV = fisheye_transform( UV, i );
                
                WeightedUV w_uv;
                w_uv.dist = outer_radius - dist;
                if( w_uv.dist == 0 ){
                    //return vec3( 1, 1, 1);
                }
                w_uv.UV = new_UV;
                
                results[count] = w_uv;
                count++;
                fisheye = true;
            }
        }
    }
    
    if( count == 0 ){
        sum = UV;
    }
    else{
        colorize = 1.0f;
        // get total summed distance
        float total_dist = 0.0f;
        for( int i = 0; i < count; i++ ){
            if( results[i].dist == 0 ){
                return vec3( 1, 1, 1);
            }
            total_dist = total_dist + results[i].dist;
        }
        float num;
        for( int i = 0; i < count; i++ ){
            WeightedUV curr = results[i];
            float weight = (curr.dist) / (total_dist);
            sum = sum + curr.UV * weight;    
        }
        
    }
    vec3 result = texture( texture_sampler, sum).rgb;
    if( linear ){
        result.r = colorize;
    }
    if( fisheye ){
        result.b = colorize;
    }
    return result;
}

void main()
{
	color = get_color();
}

