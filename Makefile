<<<<<<< HEAD

---

```makefile
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
INCLUDES = -Iinclude
SRC = src/uthreads.cpp
TEST = examples/test_example.cpp
TARGET = test_example

all: $(TARGET)

$(TARGET): $(SRC) $(TEST)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

clean:
	rm -f $(TARGET)
=======

---

```makefile
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
INCLUDES = -Iinclude
SRC = src/uthreads.cpp
TEST = examples/test_example.cpp
TARGET = test_example

all: $(TARGET)

$(TARGET): $(SRC) $(TEST)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

clean:
	rm -f $(TARGET)
>>>>>>> ba6179b (Initial commit of uthreads project)
