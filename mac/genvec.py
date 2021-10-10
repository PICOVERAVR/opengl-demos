import sys, random

if len(sys.argv) != 4:
	print("Usage: genvec <min> <max> <num>");
	sys.exit(-1)

min = float(sys.argv[1])
max = float(sys.argv[2])
runs = int(sys.argv[3])
for i in range(1, runs-1): #todo: implement vec4 and mat4 and stuff
	print("glm::vec3( %f, %f, %f)," % (random.uniform(min, max), random.uniform(min, max), random.uniform(min, max)))
print("glm::vec3( %f, %f, %f)" % (random.uniform(min, max), random.uniform(min, max), random.uniform(min, max)))
