# Local config file for ramcloud that overrides settings in config.py.

from config import *

import socket
import os


machines = [6, 8, 9, 10, 11, 12] # replace 

user = 'alice' # replace
hosts = []

for machine in machines:
  host = 'rc%d.example.com' %(machine) # replace
  ip = socket.gethostbyname(host)
  eid = ip.split('.')[-1]
  hosts.append(('%s@%s'%(user, host), ip, eid))

recovery_host = ('recovery.example.com') # replace
recovery_ip = socket.gethostbyname(recovery_host)
recovery_user = 'bob' # replace

old_master_host = ('%s@%s' %(recovery_user, recovery_host), recovery_ip, 81)

obj_dir = 'obj.master'

scripts_path = '/opt/ryan/RAMCloud/scripts'

remote_wd = '/opt/ryan/RAMCloud'

top_path = os.path.abspath(scripts_path + '/..')

# Full path to the directory containing RAMCloud executables.
obj_path = '%s/%s' % (top_path, obj_dir)

default_disk1 = '-f /opt/ryan/data/ramcloud/backup.dat'

default_disk2 = None
