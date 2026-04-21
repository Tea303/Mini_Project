CXX = g++
CXXFLAGS = -std=c++17 -Wall -g

TARGET = query_engine
SRCS = main.cpp CSVParser.cpp StorageManager.cpp
HEADERS = BPlusTree.h BPlusTreeNodes.h CSVParser.h StorageManager.h
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) *_pi.db