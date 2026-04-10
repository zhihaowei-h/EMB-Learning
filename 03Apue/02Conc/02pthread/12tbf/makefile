LDFLAGS += -pthread
SRC = main.o tbf.o
OBJ = target
$(OBJ) : $(SRC)
	gcc -o $@ $^ $(LDFLAGS)
clean :
	rm -rf $(OBJ) $(SRC)