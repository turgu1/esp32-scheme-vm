
FLAGS = 

TARGET = scheme

OBJS = \
  main.o \
  mm.o

all : $(TARGET)

REBUILDABLES = $(OBJS) $(TARGET) *.dep

clean :
  rm -f $(REBUILDABLES)

$(TARGET) : $(OBJS)
  gcc -g -o $@ $^

%.o : %.c
  gcc -g -o $@ -c $<

%.dep : %.c
  gcc -M $(FLAGS) $< > $@

include $(OBJS:.o=.dep)


