FROM centos:7

RUN yum -y install make gcc gcc-c++ wget curl curl-devel
RUN yum -y install epel-release 
RUN yum -y install python36 python36-devel python36-pip
RUN yum -y install openssl openssl-devel glib2 glib2-devel \
 json-c json-c-devel
ARG INSTALL_DIR=/usr/local/src/syslog-ng
# Which version Install Then Add to which tard dir
WORKDIR ${INSTALL_DIR}
RUN wget https://github.com/balabit/syslog-ng/releases/download/syslog-ng-3.22.1/syslog-ng-3.22.1.tar.gz && \
  tar zxvf syslog-ng-3.22.1.tar.gz
RUN cd syslog-ng-3.22.1 && ./configure --prefix=/software/syslog-ng \
--enable-http \
--enable-python \
--with-python=3 && \
make && make install 
RUN rm -rf ${INSTALL_DIR}

# Add python-path 
RUN echo 'export PYTHONPATH=$PYTHONPATH:/software/syslog-ng/intercepters' >> /etc/profile

CMD ['/software/syslog-ng/sbin/syslog-ng', '-F']
