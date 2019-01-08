FROM python:3-alpine

WORKDIR /app

COPY tools/web.py /app/

RUN pip3 install --no-cache-dir requests fake-useragent

EXPOSE 5000

CMD [ "python", "./web.py" ]