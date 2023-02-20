FLAGS = -std=c++17 -pthread #-masm=intel -fconcepts -mlong-double-128 -Wpedantic -Wall -Wextra -Wconversion -Wsign-conversion -Weffc++ -Wstrict-null-sentinel -Wold-style-cast -Wnoexcept -Wctor-dtor-privacy -Woverloaded-virtual -Wsign-promo -Wzero-as-null-pointer-constant -Wsuggest-final-types -Wsuggest-final-methods -Wsuggest-override -lquadmath

generator:
	g++ $(FLAGS) main.cpp ../utils/sha256.cpp -o $@

debug:
	rm -f debug
	g++ $(FLAGS) main.cpp ../utils/sha256.cpp -o $@ -g

run: generator
	./$<

clean:
	rm -f generator
	rm -f debug
	rm -f *.csv*
