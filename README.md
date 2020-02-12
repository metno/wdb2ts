WDB2TS
======

Wdb2ts is part of the wdb system. WDB is a databasesystem to store meteorological data.
For more information on WDB have a look at wdb.met.no. 

Wdb2ts is a simple front end that can be used to retrive meteorological data for a 
specific location from a wdb database.

It is implemented as a apache module and use a REST like interface to request data.

For help on the request string point yours favorite browser at the manual.html in the 
doc directory. If you have installed wdb2ts point the browser at http://your.site/wdb2ts.

See INSTALL for generic installation instructions.

INSTALL WDB2TS
--------------

Wdb2ts is implemented as an Apache module. It depends on Apache version 2.x. 
It is tested an developed on version 2.2.x with the threaded mpm_worker_module. 
It should work with different MPM module too.

In order to compile wdb2ts the development files for Apache must be installed. The
configure script use apxs, as is part of the apache installation, to find information
about your installation of Apache. On some systems the name of this program is apxs2.
This is true for the Apache 2.x on debian.

Other dependencies.
- Boost  1.44.0
- putools (metlibs)
- pgconpool  (metlibs)
- milib (metlibs)
- libmi (metlibs)
- pumet (metlibs)
- puctools (metlibs)
- curl
- postgresql client library.
- proj >= 4.4.9
- libpqxx >= 2.6.x.

Boost
----- 
Boost can be downloaded from www.boost.org. From the library you must install
the librarys thread, iostream, date_time, system , filesystem and 
regex in addition to the base.
  
postgresql
----------
Postgresql can be downloaded from www.postgresql.org. 

libpqxx
-------
A C++ client library for postgresql. It can be down loaded from
http://pqxx.org/.
  
 
wdb2ts
------
To compile wdb2ts. 

```shell
$ ./configure
```

If the name of the apxs proram is apxs2 or something else `--with-apxs` must be
specified. `--with-apxs` may also be used if configure doesn't find _apxs_. 

E.g. if the apxs is located at `/usr/bin/apxs2`, then run configure with:

```shell
$ ./configure --with-apxs=/usr/bin/apxs2
```

Other useful options are:

```shell
--with-boost, where boost is installed.
--with-pgsql, where the postgresql client librarys is installed.
--with-wdb2ts-configdir, default the configuration files is installed
         in the Apache conf directory as given with the `apxs -q SYSCONFDIR`. 
         This can be overided with --with-wdb2ts-configdir. 
```

For more options and help type `./configure --help`.

Compile with:

```shell
$ make
```

After a successful compilation the module is installed with 

```shell
$ make install
``` 

You might need to `make install` as root :-). 

The install process installs the file mod_metno-wd2ts.so in `apxs -q LIBEXECDIR`.
In addition is some configuration files that can be used as template installed in
`apxs -q SYSCONFDIR` or in the location given wit `--with-wdb2ts-configdir`. 


CONFIGURE WDB2TS
----------------

There are two configuration files that must be edited, _metno-wdb2ts.conf_ and
_metno-wdb2ts-db.conf_.

_metno-wdb2ts.conf_ configures Apache and _metno-wdb2ts-db.conf_ configures the 
connection information to the wdb database that is to be used.

metno-wdb2ts.conf
-----------------

The contents of the metno-wdb2ts.conf.template file are something like:

```xml
LoadModule metno_wdb2ts_module modules/mod_metno-wdb2ts.so

LogLevel info

<Location /wdb2ts>
   SetHandler metno-wdb2ts
</Location>
```

This loads the wdb2ts module into apache and defines the url path that
gets sent to the module, in this case _"/metno-wdb2ts"_. 

For this file to take effect it must be read by apache at upstart. The easy
way to do this is to reference it in http.conf with the Include directive.

E.g. in _http.conf_ add:

```
Include "/path/to/metno-wdb2ts.conf"
```

followed by a restart of Apache with `apachectl restart`.
   

metno-wdb2ts-db.conf
--------------------

This file tells the module which wdb database to use.

The format is:

```shell
name: default=true host=wdb.my.site user=wdb port=5432 dbname=mywdb passwors=secret
```

- **name**: an arbitary name you give the connection, not used at the moment.
- **default**: use as the default database or not.
- **host**: server the database is on.
- **user**: user to connect to the database as. 
- **port**: port the database is located on, on the server.
- **dbname**: name of the database.
- **password**: to login to the database mywdb


