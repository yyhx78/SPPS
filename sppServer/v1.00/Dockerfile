FROM yyhx78/spps_base

USER root

WORKDIR /home/yyhx78

COPY ./apps/bin/sppServer /usr/local/bin/sppServer
COPY ./apps/bin/startSPPS /usr/local/bin/.
COPY ./apps/bin/foamToOFRV /usr/local/bin/.
COPY ./apps/www /home/yyhx78/www

EXPOSE 7777

RUN ["chmod", "+x", "/usr/local/bin/startSPPS"]

ENTRYPOINT ["/usr/local/bin/startSPPS"] 

USER yyhx78

# to build the image: docker build . -t yyhx78/spps:1.00
# to run the server: docker run -itd -v $(pwd):/home/yyhx78/cases -p 7779:7777 yyhx78/spps:1.00 /bin/bash
# to push to the hub: docker push yyhx78/spps:1.00
