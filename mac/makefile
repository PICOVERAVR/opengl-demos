CC := clang++
CFLAGS := -Wall -o OpenGL.out

SRC := main.cpp load.cpp # fix this eventually please
DSRC := diag.cpp

PKGSRC := `pkg-config glfw3 --cflags`
PKGLIB := `pkg-config glfw3 --libs`

opengl: 
	@$(CC) $(PKGSRC) $(SRC) $(CFLAGS) -framework OpenGL  $(PKGLIB)
	@echo "OpenGL build completed."

diag:
	@$(CC) $(PKGSRC) $(DSRC) $(CFLAGS) -framework OpenGL $(PKGLIB)
	@echo "OpenGL diag build completed."

clean:
	@rm -f *.out
	@rm -f *.o
