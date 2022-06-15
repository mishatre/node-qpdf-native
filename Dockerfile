
FROM node:17-alpine3.14
WORKDIR /usr/src/app

COPY . .

RUN apk add --no-cache \
    # python3 pkgconf make g++ \
    qpdf-dev 
    # && npm install 
    # \
    # && apk del .gyp

# RUN npm install -g node-gyp prebuildify

# RUN prebuildify --t 17.8.0 --libc musl --tag-libc --strip

RUN npm run install

ENTRYPOINT ["tail", "-f", "/dev/null"]