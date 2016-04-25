COMPILER      = clang
WARNINGS      = -Weverything -Wno-padded -Wno-cast-align -Wno-float-equal -Wno-format-nonliteral -Wno-sign-conversion -Wno-conversion -Wno-unused-parameter
COMPILE_FLAGS = -std=c11
LIBRARIES     = -lglfw3 -lm
LIBRARIES_OS  = -framework OpenGL -framework Cocoa -framework IOKIT -framework CoreVideo

SOURCES = \
	../shared/library/stb_image.c \
	../shared/texture.c           \
	../shared/map.c               \
	font.c    \
	raycast.c \
	render.c  \
	handle.c  \
	main.c

OBJECTS = $(SOURCES:.c=.o)
TARGET  = wolfcast

all: $(TARGET)

# No Warnings, Other Std, Headers Affect Objects
../shared/library/stb_image.o: ../shared/library/stb_image.c ../shared/library/stb_image.h
	$(COMPILER) -std=c89 $(COMPILE_FLAGS) -c ../shared/library/stb_image.c -o $@

%.o: %.c
	$(COMPILER) $(COMPILE_FLAGS) $(WARNINGS) -c $^ -o $@

$(TARGET): $(OBJECTS)
	$(COMPILER) $(OBJECTS) $(LIBRARIES) $(LIBRARIES_OS) -o $(TARGET)

clean:
	rm -f $(OBJECTS)

veryclean: clean
	rm -f $(TARGET)

remake: veryclean $(TARGET)
