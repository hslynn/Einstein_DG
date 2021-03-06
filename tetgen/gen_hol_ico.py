"""
Prints a hollowed sphere as an hollowed isohedron - a regular polyedron with 20 identical
equilateral triangular faces, 30 edges and 12 vertices.
Code was modified from:
http://www.andrewnoske.com/wiki/Generating_a_sphere_as_a_3D_mesh
"""
RADIUS_EXT = 10.0
RADIUS_IN = 1.8
NUM_VERTS = 12
BASE_VERTS = [[ 0.000,  1.000,  0.000],   
                [ 0.894,  0.447,  0.000],
                [ 0.276,  0.447,  0.851],
                [-0.724,  0.447,  0.526],
                [-0.724,  0.447, -0.526],
                [ 0.276,  0.447, -0.851],
                [ 0.724, -0.447,  0.526],
                [-0.276, -0.447,  0.851],
                [-0.894, -0.447,  0.000],
                [-0.276, -0.447, -0.851],
                [ 0.724, -0.447, -0.526],
                [ 0.000, -1.000,  0.000]]
BASE_FACETS = [[1, 2, 3],   
            [1, 3, 4],
            [1, 4, 5],
            [1, 5, 6],
            [1, 6, 2],
            [12, 7, 8],
            [12, 8, 9],
            [12, 9, 10],
            [12, 10, 11],
            [12, 11, 7],
            [2, 3, 7],
            [3, 4, 8],
            [4, 5, 9],
            [5, 6, 10],
            [6, 2, 11],
            [7, 8, 3],
            [8, 9, 4],
            [9, 10, 5],
            [10, 11, 6],
            [11, 7, 2]]      
int_verts = [list(map(lambda n:n*RADIUS_IN, vert)) for vert in BASE_VERTS]
ext_verts = [list(map(lambda n:n*RADIUS_EXT, vert)) for vert in BASE_VERTS]
ext_facets = [list(map(lambda n:n+NUM_VERTS, face)) for face in BASE_FACETS]
verts = int_verts + ext_verts;
facets = BASE_FACETS + ext_facets;

print(verts)
fn = open("hollowed_icosahedron.poly","w" )

# PRINT VERTICES:
fn.write("# node count, dim, num attribute, boundary marker\n")  
fn.write(str(NUM_VERTS*2) + " 3 0 0\n")
for idx in range(len(verts)):
    vert = verts[idx]
    line = (str(idx + 1) + "  " + "  ".join([str(coord) for coord in vert]) +
        "  0  0" + "\n")
    fn.write(line)

# PRINT FACETS:
fn.write("# num facets, boundary markers\n")  
fn.write(str(len(facets)) + " 1\n")

for face in BASE_FACETS:
    fn.write("1 0 0\n")
    fn.write(str(len(face)) + " " + " ".join([str(vert) for vert in face]) + "\n")

for face in ext_facets:
    fn.write("1 0 1\n")
    fn.write(str(len(face)) + " " + " ".join([str(vert) for vert in face]) + "\n")

fn.write("# num holes\n")  
fn.write("1\n")  
fn.write("1 0.0 .0.0 0.0\n")  

fn.close()
