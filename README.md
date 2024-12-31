# web-server
this is implemented in C++

# Build Your Own Web Server
This challenge is to build your own basic web server.

At its core a web server is actually quite simple. It’s a server that listens for connections from clients and responds to them. The clients make those requests using a protocol known as HTTP (and expect responses in the same protocol, obviously).

HTTP like many early Unix and Internet protocols is text based, so human readable. The original HTTP specification from 1991, is short and sweet. It originally didn’t even have a version number! But was later renamed HTTP/0.9, to differentiate it from HTTP/1.0. The HTTP/1.0 specification seems to be lost in the mists of time.

The first full formal HTTP specification is HTTP/1.1 - also known as - RFC2616 from 1999. You can dig through all the HTTP standards on the W3C website.

The Challenge - Building a Basic Web Server
Early web servers were very basic, as per the HTTP/0.9 specification they supported just a GET request and returned the document specified. The error messages were human readable HTML with no way to distinguish success from failure.

We’re going to go a little beyond that and support a small subset of HTTP/1.1.

## Step Zero
In this step you decide which programming language and IDE you’re going to use and you get yourself setup with a nice new ‘webserver’ project. I built mine in Rust.

## Step 1
In this step your goal is to create a basic HTTP server that listens on port 80 and can handle a single TCP connection at a time. For all requests we’ll return some text that describes the requested path.

For example if our server is running locally, a client might make the curl request below and get back the simple text message.

curl http://localhost/
Requested path: /

To support this your server will need to create a socket, and bind it to the address of your server. For the purposes of this challenge that can be the IP address: 127.0.0.1 (the loopback address, also known as localhost) and port: 80 (the default HTTP port).

Once that is done, your server will need to listen for requests, accept incoming requests and then receive the incoming data.

You can learn more about sockets in the Wikipedia article on Berkeley Sockets. Your programming language probably provides a wrapper around this API in its standard library for example Python has socket, Rust has std::net and node has node:net.

For an in-depth look at network programming check out Beej’s Guide to Network Programming.

Once you can receive data from the client you’ll need to parse that data to extract the key elements. For this step that is simply taking the first line of the client request, which will look something like this:

GET / HTTP/1.1

From which you’ll need to recognise that this is the Request value, the type of request is GET, the requested resource is / and the HTTP version is 1.1.

For this step, you’ll need to return the bare minimum HTTP response:

HTTP/1.1 200 OK\r\n\r\nRequested path: <the path>\r\n

Which you will do by sending that data back over the socket. Once you have that working we have a very basic server, but it’s not much use yet. Don’t forget to close the socket when you’re done sending data.

## Step 2
In this step your goal is to server your first HTML document. When the request is for a valid path, your web server should return the document at that path and the HTTP status code 200 as in step 1.

So first, lets create a simple HTML test page, something like this:

<!DOCTYPE html>
<html lang="en">
  <head>
    <title>Simple Web Page</title>
  </head>
  <body>
    <h1>Test Web Page</h1>
    <p>My web server served this page!</p>
  </body>
</html>

Save this as index.html in your project, perhaps in a www directory. Then change your server to use the path we extracted from the HTTP request. Open the file at that path and return it’s contents after the HTTP header for success (HTTP/1.1 200 OK\r\n\r\n).

By convention a request for / usually serves up the file index.html, we’ll follow that, so a request to http://localhost/ or http://localhost/index.html would return the same document.

So when you test you should now see:

% curl -i http://localhost/
HTTP/1.1 200 OK

<!DOCTYPE html>
<html lang="en">
  <head>
    <title>Simple Web Page</title>
  </head>
  <body>
    <h1>Test Web Page</h1>
    <p>My web server served this page!</p>
  </body>
</html>

or:

% curl -i http://localhost/index.html
HTTP/1.1 200 OK

<!DOCTYPE html>
<html lang="en">
  <head>
    <title>Simple Web Page</title>
  </head>
  <body>
    <h1>Test Web Page</h1>
    <p>My web server served this page!</p>
  </body>
</html>

Assuming you used the example HTML from above.

When the request is for an invalid path, you server should return the status code 404 and a suitable error message, the usual is Not Found.

% curl -i http://localhost/invalid.html
HTTP/1.1 400 Not Found

Once you’ve got that working, pause for a moment and think about the security risks you might have introduced in your simple server.

## Step 3
In this step your goal is to handle multiple clients at a time, after all in the real world we don’t want a web server that can only handle one client at a time - that’s just not going to scale.

We want to be able to handle multiple concurrent clients, binding each incoming connection to a new socket and then receiving and sending data over that socket connection. We don’t want one connection to block another so we’ll need some support for concurrent operations in our server.

Traditionally this was handled by creating a new thread for each connection, more recently there has been a focus on using asynchronous frameworks, this boils down to there being an overhead to threads. If you’ve never used threads now would be a great time to try it out. If you have then perhaps try the async framework for your stack of choice. There’s an overview of parallelism, concurrency, and asynchrony on the Coding Challenges blog.

You can then test your server by sending it multiple concurrent requests. If like me you really want to see what’s happening you could make your connection handling thread log it’s id and then sleep 20 seconds before sending the HTML back. That’s what I did and then sent multiple requests with curl and see my concurrency in action:

Here’s the server, with 7 connection handling threads all going:

Path: index.html
Thread Id: ThreadId(2)
Path: index.html
Thread Id: ThreadId(3)
Path: index.html
Thread Id: ThreadId(4)
Path: index.html
Thread Id: ThreadId(5)
Path: index.html
Thread Id: ThreadId(6)
Path: index.html
Thread Id: ThreadId(7)
Path: index.html
Thread Id: ThreadId(8)

And here’s my simple test clients:

% curl -i http://localhost/ &
[1] 33430
% curl -i http://localhost/ &
[2] 33438
% curl -i http://localhost/ &
[3] 33446
% curl -i http://localhost/ &
[4] 33454
% curl -i http://localhost/ &
[5] 33462
% curl -i http://localhost/ &
[6] 33470
% curl -i http://localhost/ &
[7] 33478

20 seconds after the first request the HTML started coming back from the server.

So by now you have a basic web server that can handle multiple concurrent client requests.

## Step 4
In this step your goal is to address some of the issues you hopefully thought of at the end of Step 2. Namely at the moment our server will open a valid file and stream it back to the client. So if we’re not careful they could provide a path to a file we don’t want to share, i.e. /etc/passwd on a Unix like system.

Your task for this part of the challenge is to ensure that whatever request path is provided by the client, you only serve documents that are in the www directory or it’s subdirectories.

Ideally your server should allow you to specify the location of the of www folder on startup. Devise some test cases and give it a go!

Once you’ve done that, congratulations 🎉  you’ve built a basic web server that can serve HTML documents!
