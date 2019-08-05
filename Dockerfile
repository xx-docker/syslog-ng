FROM centos:7

RUN yum -y install make gcc gcc-c++ 
RUN yum -y install epel-release 
RUN yum -y install python36 python36-devel python36-pip
RUN yum -y install openssl openssl-devel glib2 glib2-devel \
 json-c json-c-devel
ARG INSTALL_DIR=/usr/local/src/syslog-ng
# Which version Install Then Add to which tard dir
ADD . ${INSTALL_DIR}
RUN cd ${INSTALL_DIR} && ./configure --prefix=/software/syslog-ng \
--enable-http \
--enable-python \
--with-python=3 && \
make && make install 
RUN rm -rf ${INSTALL_DIR}

CMD ['/software/syslog-ng/sbin/syslog-ng', '-F']
