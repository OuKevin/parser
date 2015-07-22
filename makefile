all: truc
truc:token.o idtoken.o keywordtoken.o punctoken.o reloptoken.o addoptoken.o muloptoken.o numtoken.o eoftoken.o buffer.o scanner.o parser.o truc.o symbol_table.o register.o register_allocator.o emitter.o
	g++ -o truc token.o idtoken.o keywordtoken.o punctoken.o reloptoken.o addoptoken.o muloptoken.o numtoken.o eoftoken.o buffer.o scanner.o symbol_table.o truc.o parser.o register.o register_allocator.o emitter.o

token.o: token.cc token.h
	g++ -c token.cc

idtoken.o: token.h idtoken.cc idtoken.h
	g++ -c idtoken.cc

keywordtoken.o: token.h keywordtoken.cc keywordtoken.h
	g++ -c keywordtoken.cc

punctoken.o: token.h punctoken.cc punctoken.h
	g++ -c punctoken.cc

reloptoken.o: token.h reloptoken.cc reloptoken.h
	g++ -c reloptoken.cc

addoptoken.o: token.h addoptoken.cc addoptoken.h
	g++ -c addoptoken.cc

muloptoken.o: token.h muloptoken.cc muloptoken.h
	g++ -c muloptoken.cc

numtoken.o: token.h numtoken.cc numtoken.h
	g++ -c numtoken.cc

eoftoken.o: token.h eoftoken.cc eoftoken.h
	g++ -c eoftoken.cc

buffer.o: buffer.cc buffer.h
	g++ -c buffer.cc

scanner.o: scanner.cc scanner.h buffer.h token.h keywordtoken.h punctoken.h reloptoken.h addoptoken.h muloptoken.h idtoken.h numtoken.h eoftoken.h
	g++ -c scanner.cc

test_scanner.o: test_scanner.cc scanner.h
	g++ -c test_scanner.cc
	
parser.o: parser.cc parser.h token.h keywordtoken.h punctoken.h reloptoken.h addoptoken.h muloptoken.h idtoken.h numtoken.h eoftoken.h scanner.h symbol_table.h register_allocator.h register.h emitter.h
	g++ -c parser.cc
	
symbol_table.o: symbol_table.cc symbol_table.h emitter.h
	g++ -c symbol_table.cc
	
truc.o: truc.cc parser.h
	g++ -c truc.cc
	
register.o: register.cc register.h
	g++ -c register.cc

register_allocator.o: register_allocator.cc register_allocator.h
	g++ -c register_allocator.cc

emitter.o: emitter.cc emitter.h
	g++ -c emitter.cc

clean:
	rm -rf *o truc
