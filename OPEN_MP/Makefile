CC = icpc
CFLAGS = 
COPTFLAGS = -O3 -g 
LDFLAGS =

default:
	@echo "=================================================="
	@echo "To build your sorting code, use:"
	@echo "  make mergesort-omp    	 # For Mergesort"
	@echo ""
	@echo "To clean this subdirectory (remove object files"
	@echo "and other junk), use:"
	@echo "  make clean"
	@echo "=================================================="

# Mergesort driver
mergesort-omp: driver.o sort.o mergesort-omp.o
	$(CC) $(COPTFLAGS) -o $@ $^

%.o: %.cc
	$(CC) $(CFLAGS) $(COPTFLAGS) -o $@ -c $<

clean:
	rm -f core *.o *~ mergesort-omp

# eof
