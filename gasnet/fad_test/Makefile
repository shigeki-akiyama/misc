include /home/usr1/12IAH358/local/opt/ompi-1.8.3/include/ibv-conduit/ibv-seq.mak  
# -I/home/usr1/12IAH358/local/opt/ompi-1.8.3/include/ibv-conduit 

a.out : fad_test.o
	mpicxx -o $@ $< $(GASNET_LIBS)
#	mpicxx $(GASNET_LDFLAGS) -o $@ $< $(GASNET_LIBS)

fad_test.o: fad_test.cc
	mpicxx -g -O0 $(GASNET_CPPFLAGS) -c $<
#	$(GASNET_CXX) $(GASNET_CPPFLAGS) $(GASNET_CXXFLAGS) -c $<

.PHONY: clean
clean:
	rm -f fad_test.o a.out
