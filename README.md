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


# Schedule
