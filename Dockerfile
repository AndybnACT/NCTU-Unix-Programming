FROM fedora
RUN yum -y update
RUN yum -y install git
RUN dnf -y groupinstall 'Development Tools'
RUN dnf -y install yasm 

COPY ./HWs /home/HWs
WORKDIR /home/HWs/
RUN make
RUN make test
