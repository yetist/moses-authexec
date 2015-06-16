#!/bin/bash
USER=`whoami`
USERID=`id`
echo $USERID > /tmp/$USER
pwd >> /tmp/$USER
