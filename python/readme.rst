Python and REST API
##########################################
Jack Lee  April 5th, 2019


Support new content-type

#define		HTTP_HDR_SDP					"application/octet-stream"
#define		HTTP_HDR_SDP_EXT				"application/sdp"



Installation
=============================
::

   sudo apt-get install python-pip

   sudo pip install virtualenv

   virtualenv env

   . env/bin/activate

   pip install Flask

   python -m flask.config

simplest web service:
   python hello.py 

flash rest module
::

   pip install flask-restful
   
   pip freeze > requirements.txt
   
   python rest/app.py


Debugging
==============================

* Restarting with stat : No such file or directory
--------------------------------------------------------
::

    app.run(debug=True)
    
    Change as following:
    app.run(debug=False)

    app.run(use_reloader=False)


* OSError: [Errno 8] Exec format error
--------------------------------------------------------
add following line in the first line of python file:
::

   #!/usr/bin/env python


