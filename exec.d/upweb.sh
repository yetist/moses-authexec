#!/bin/bash
echo $HOME > /tmp/log
whoami >> /tmp/log
id >> /tmp/log
pwd >> /tmp/log
exit 0
deploy_base="/www/python/django/`whoami`"
git_url='git://git.meegobox.org/address_app.git'

if [ ! -d $deploy_base/address_app ]; then
        git clone $git_url $deploy_base
else
	cd $deploy_base/address_app/address/
        git fetch && git reset --hard origin/master
        python manage.py collectstatic --noinput
        python manage.py syncdb
        python manage.py migrate
        touch address/wsgi.py
fi

deploy_base="/www/python/django/address.meegobox.org"

cd $deploy_base/address_app/address/
chown web21:web21 -R .
chmod 770 -R media
#if [ -d appstore/djangobb_index ];then
#    chmod 770 -R appstore/djangobb_index
#fi
/etc/init.d/nginx force-reload && /etc/init.d/uwsgi restart
