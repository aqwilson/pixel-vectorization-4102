# Pixel Art Vectorization for COMP 4102
Alexandra Wilson, Mitchell Blanchard, Nathan Marshall, and Megan Perera

# Summary
Pixel art is difficult to upscale using traditional vectorization methods due to its small scale and the fact that each pixel is meaningful, as traditional vectorization methods rely on edge detection that fails in smaller sample sizes. This project seeks to implement and experiment with the algorithm for pixel art vectorization proposed by Kopf and Lischinski in the paper *Depixelizing Pixel Art (SIGGRAPH 2011)*. Challenges may include improving or developing new heuristics for pixel-art specific edge detection and/or optimizing the splines used to create smoother lines.

# Background 
This project is based on a paper called *Depixelizing Pixel Art* by Kopf and Lischinski that was published in SIGGRAPH 2011. It discussed the difficulties of pixel art vectorization, especially with traditional algorithms, and provides an alternative algorithm for vectorization. It involves selecting pixels that belong to lines, and extracting these lines into B-Splines, which can then be rendered using traditional vector-rendering methods. It relies on use of heuristics to select meaningful edge segments to minimize complex computations at runtime.

https://johanneskopf.de/publications/pixelart/

# Challenge
This is a challenging project because it requires making optimizations and departures from methods that would be standard in higher resolution vectorization. It requires the use of very specific algorithms that are not trivial to implement in OpenCV, and requires that we look for new and different heuristics to make the task of vectorization more efficient.

We are hoping to learn more about the process of vectorization, as well as about the difficulties of working with low resolution, high meaning images. 

# Goals and Deliverables
Our main goal with this project will be to implement the algorithm discussed in the paper, and as a difference, manage anti-aliasing within pixel art samples. Potential extensions, should we have time, may include addressing dithering within the pixel art samples.

Deliverable include:
* source code implementing the majority of the algorithm described in [Kopf & Lischinski, 2011] that generates an SVG image as output
* a final paper describing the algorithm and work performed
* a video presentation describing the work performed

# Schedule
* **Feb 8th:** General Project Setup. Nathan will be creating and managing a CMake file. Megan will be sourcing sample images for us and creating anti-aliased samples from them. Alex will be doing initial commits and project setup, including GitHub configuration. Mitchell will be starting work on the connected similarity graph, up to the point where we need to start removing edges.

* **February 15th:** Starting the implementation of the base algorithm. Begin by working on creating a connectivity graph. Implement 3 heuristics for connecting and disconnecting edges (the curve, sparse pixels, and island heuristics).

* **February 22nd:** Extract the reshaped cell graph through edge-cutting and computing a simplified Voronoi diagram for the cells. 

* **February 29th:** Compute the B Splines for the pixel graph. This will include identifying visible edges, setting endpoints for each spline, and choosing which splines to connect at junction points based on edge type (shading vs contour) and angle between splines.

* **March 7th:** Optimize the curves. This will include implementing fairly complex calculations for the energy of each node for smoothness and position, ensuring that sharp details are not smoothed, randomly selecting points on the graph to take the minimal energy for, and computing new positions for untouched nodes using harmonic maps. We will also work on rendering the vectorized images with colour calculated through Gaussian influence functions.  

* **March 14th:** We will finish any rendering work, and will begin working on identifying and implementing our anti-aliasing heuristics. Time will be spent implementing and optimizing these algorithms.

* **March 21st:** Continue work on identifying and implementing better heuristics for anti-aliasing detection. We will have a greater focus on optimization throughout the week, and ensuring that anti-aliasing detection is not yielding false positives for regular gradients in an image.

* **March 28th:** Write our final report, ensure that our code is well documented, and prepare supplementary materials. Work on fixing bugs and improving our implementations.

* **April 4th:** Buffer room. Dedicated time for fixing unexpected bugs and problems that may arise during development. 

* **April 15th:** Final project will be submitted by this day. 

# Results
We achieved significant results in our implementation for both anti-aliasing cleanup and converting the pixel art images to vectors. We were able to convert and dynamically scale the vectorized images. We deviated from the original algorithms in order to prevent large scale changes. Some of our results may be found in the "results" folder, and we show these results as well in our final paper.
