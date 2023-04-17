FROM gcc:latest
COPY *.c /
COPY makefile /makefile 
EXPOSE 49153
RUN make
CMD ["./server"]
