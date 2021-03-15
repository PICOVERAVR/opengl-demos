## Cascaded Shadow Mapping Example
In this demo, I implemented the basic shadow mapping algorithm, then built on it with various optimizations:

0. Offsetting the depth of vertices means we don't get shadow acne.
1. Writing into the depth texture with a linear filter means the shadow edges will look softer.
2. Writing the depth buffer with NDC coordinates instead of world-space coordinates means all shadow texels are the same size in the image plane, eliminating perspective aliasing. This is also known as Perspective Shadow Mapping (PSM).
3. Make sure the frustrum fits as tightly as possible - having the near point be too close will result in artifacts and having the far point be too far away results in performance problems.
4. Clamping the border texture to 1.0f ensures that anything outside the frustrum gets lit, which looks better.

After getting shadow mapping working, I added in regular lighting in the fragment shader. I did this because applying the shadow mapping algorithm to the teapot itself makes things look terrible.

Finally, I implemented a scaled-down version of Cascaded Shadow Mapping (CSM) that clearly illustrates the texture resolution decrease. The depth textures that are used for CSM are also displayed on the screen in the bottom left to make things look cooler. I did a compute shader pass over the depth textures to map the depth values to [0.0, 1.0] with the closest point being 0.0 and the farthest point being 1.  The time it took to execute the compute shader over 100 frames is printed out for fun.

The output of the program is included below:
![Alt text](output.png?raw=true "Example output")
