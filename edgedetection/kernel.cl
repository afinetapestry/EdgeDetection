__kernel void gaussian(read_only image2d_t input, write_only image2d_t output) {
	const int gaussian[9] = {1, 2, 1, 2, 4, 2, 1, 2, 1};
	
	const sampler_t sampler = CLK_FILTER_NEAREST | CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE;
	
	const int2 size = get_image_dim(input);
	
	int2 coord = (int2)(get_global_id(0), get_global_id(1));
	
	int2 start = (int2)(coord.x - 1, coord.y - 1);
	int2 end = (int2)(coord.x + 1, coord.y + 1);
	
	uint4 color = (uint4)(0, 0, 0, 0);
	
	for (int x = start.x; x <= end.x; ++x) {
		for (int y = start.y; y <= end.y; ++y) {
			if (x <= size.x && y <= size.y) {
				color += read_imageui(input, sampler, (int2)(x, y)) * gaussian[((y - start.y) * 3) + x - start.x];
			}
		}
	}
	
	write_imageui(output, coord, color / 16);
}

__kernel void laplacian(read_only image2d_t input, write_only image2d_t output) {
	const int laplacian[9] = {-1, -1, -1, -1, 8, -1, -1, -1, -1};
	
	const sampler_t sampler = CLK_FILTER_NEAREST | CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE;
	
	const int2 size = get_image_dim(input);
	
	int2 coord = (int2)(get_global_id(0), get_global_id(1));
	
	int2 start = (int2)(coord.x - 1, coord.y - 1);
	int2 end = (int2)(coord.x + 1, coord.y + 1);
	
	uint4 color = (uint4)(0, 0, 0, 0);
	
	for (int x = start.x; x <= end.x; ++x) {
		for (int y = start.y; y <= end.y; ++y) {
			if (x <= size.x && y <= size.y) {
				color += read_imageui(input, sampler, (int2)(x, y)) * laplacian[((y - start.y) * 3) + x - start.x];
			}
		}
	}
	
	write_imageui(output, coord, color);
}

__kernel void threshold(read_only image2d_t input, write_only image2d_t output, int value) {
	const sampler_t sampler = CLK_FILTER_NEAREST | CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE;
	
	const int2 size = get_image_dim(input);
	
	int2 coord = (int2)(get_global_id(0), get_global_id(1));
	
	if (coord.x < size.x && coord.y < size.y) {
		uint4 color = read_imageui(input, sampler, coord);
		if (((color.x * color.y * color.z) / 3) < value) {color = 0;} else {color = 255;}
		write_imageui(output, coord, color);
	}
}
