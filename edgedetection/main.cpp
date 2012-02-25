//
//  main.cpp
//  edgedetection
//
//  Created by Jonathan Hatchett on 18/02/2012.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#define __CL_ENABLE_EXCEPTIONS

#define USEGPU 0

#include <iostream>

#include <cl.hpp>

#include "loadtext.h"
#include "clpng.h"
#include "clerr.h"

int main (int argc, const char * argv[])
{
	try {
		if (argc < 3) {fprintf(stderr, "Usage: edgedetection input.png output.png\n"); return EXIT_FAILURE;}
		
		cl::Context context(USEGPU ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU);
		std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
		
		std::string programStr;
		LoadText("kernel.cl", programStr);
		
		cl::Program::Sources source(1, std::make_pair(programStr.c_str(), programStr.size()));
		cl::Program program = cl::Program(context, source);
		try {
			program.build(devices);
		} catch (cl::Error &e) {
			std::cerr << "Caught Exception: " << e.what() << std::endl << _clGetErrorMessage(e.err()) << std::endl;
			std::string str = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]);
			std::cerr << str << std::endl;
		}
		
		cl::Kernel gaussian(program, "gaussian", NULL);
		cl::Kernel laplacian(program, "laplacian", NULL);
		cl::Kernel threshold(program, "threshold", NULL);
		
		cl::Event event;
		cl::CommandQueue queue(context, devices[0], 0, NULL);
		
		unsigned int width, height;
		unsigned char *input, *output;
		if (LoadPNG24(&input, argv[1], &width, &height)) {throw "PNG load failed";}
		if (CreatePNG24(&output, width, height)) {throw "PNG create failed";}
		
		cl::size_t<3> origin; origin[0] = 0; origin[1] = 0; origin[2] = 0;
		cl::size_t<3> region; region[0] = width; region[1] = height; region[2] = 1;
		
		cl::ImageFormat imageFormat(CL_RGBA, CL_UNORM_INT8);
		cl::Image2D * a = new cl::Image2D(context, CL_MEM_READ_WRITE, imageFormat, width, height, 0, NULL, NULL);
		cl::Image2D * b = new cl::Image2D(context, CL_MEM_READ_WRITE, imageFormat, width, height, 0, NULL, NULL);
		cl::Image2D * tmp;
		
		queue.enqueueWriteImage(*a, TRUE, origin, region, NULL, NULL, input);
		
		// Begin execution
		gaussian.setArg(0, *a);
		gaussian.setArg(1, *b);
		queue.enqueueNDRangeKernel(gaussian, cl::NullRange, cl::NDRange(width, height), cl::NullRange, NULL, &event);
		event.wait();
		tmp = a; a = b; b = tmp;
		
		laplacian.setArg(0, *a);
		laplacian.setArg(1, *b);
		queue.enqueueNDRangeKernel(laplacian, cl::NullRange, cl::NDRange(width, height), cl::NullRange, NULL, &event);
		event.wait();
		tmp = a; a = b; b = tmp;
		
		threshold.setArg(0, *a);
		threshold.setArg(1, *b);
		threshold.setArg(2, 240);
		queue.enqueueNDRangeKernel(threshold, cl::NullRange, cl::NDRange(width, height), cl::NullRange, NULL, &event);
		event.wait();
		tmp = a; a = b; b = tmp;
		// End execution
		
		queue.enqueueReadImage(*a, TRUE, origin, region, NULL, NULL, output);
		
		if (WritePNG24(&output, argv[2], width, height)) {throw "PNG write failed";}
		
		ClosePNG24(&input);
		ClosePNG24(&output);
		
		delete a;
		delete b;
	} catch (cl::Error &e) {
		std::cerr << "Caught Exception: " << e.what() << std::endl << _clGetErrorMessage(e.err()) << std::endl;
	}
	
	return EXIT_SUCCESS;
}
