This software allows the user to inpaint an image using a texture synthesis based method.
This implementation is based on "Object Removal By Exemplar Based Inpainting" by A. Criminisi.

License: See LICENSE file in base directory.

Implementation notes:
Throughout the code, the size of square patches are described by their "radius". That is, a 9x9 square patch has radius 4.

In the computation of the 'priority', there is an 'alpha' in the 'data' term and a '|psi|' in the 'confidence' term.
Since the priority is the product of the 'data' and 'confidence', the constants |psi| and 'alpha' do not affect the computations at all.

