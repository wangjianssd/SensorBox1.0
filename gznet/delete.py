# -*- coding: utf-8 -*-
# in order to delte the files that with extendsnames
# 任务：需要对指定格式的文件进行批量删除，如''exe",".swf"等等。
# 
# 步骤：
#     1. 首先找到文件所在的目录。
# 
#      2. 对目录下的文件格式进行筛选.
# 
#    3. 删除文件，并有"删除成功的"文字提示.
# 
# 需要用到的函数：
# 
#      os.getcwd()           #获取当前目录
#      os.chdir(path)        #跳转到指定格式的文件所在目录
# 
#      os.lisdir(dirname)    #列出目录下的文件名及文件长度
#      os.remove(filename)   #指定需被删除的文件名称并删除文件
# 
# 自定义的:
# 
#      loadSWF2(lis,extdsname)    #对文件列表进行筛选,并返回
# 
#      getFoundfiles(strpath)     #获取被删除的文件列表
#
#      goodremove(ipath,filelist) #进行删除文件的操作

import os
import sys

# 默认要删除的文件后缀
delete_suffix = ['.orig', '.o', '.d', '.lst', '.elf', '.hex', '.map', '.bin']
proj_suffix = ('.ewp','ewd','.xcl')
ignore_suffix = ('.hg')

# 刪除文件与目录
def delete_all_file_folder(src):
    '''删除项目所在文件夹里的垃圾文件'''
    if os.path.isfile(src):
        try:
            os.remove(src)
            print "delete file :%s" %src
        except:
            pass
    elif os.path.isdir(src):
        for item in os.listdir(src):
            itemsrc=os.path.join(src,item)
            delete_all_file_folder(itemsrc) 
        try:
            os.rmdir(src)
        except:
            pass

def get_current_dir():
    return os.getcwd()

def delelte_proj_rubbish(folder):
    all_list = os.listdir(folder)

    for item in os.listdir(folder):
        itemsrc=os.path.join(folder,item)
        if itemsrc.endswith(proj_suffix):
            all_list.remove(item)

    for item in all_list:
            itemsrc=os.path.join(folder,item)
            #print itemsrc
            delete_all_file_folder(itemsrc)


def delete_file_folder(ipath):
    if os.path.isfile(ipath):
        if ipath.endswith(tuple(delete_suffix)):
            try:           
                os.remove(ipath)
                print "delete file :%s" %ipath
            except:
                pass

    elif os.path.isdir(ipath):
        for item in os.listdir(ipath):
            if item != ignore_suffix:
                itemsrc=os.path.join(ipath,item)
                # print itemsrc
                print ".",
                if os.path.isfile(itemsrc):
                    if itemsrc.endswith(tuple(proj_suffix)):
                        delelte_proj_rubbish(ipath)
                    else:
                        delete_file_folder(itemsrc)
                else:
                    delete_file_folder(itemsrc)



if __name__ == '__main__':
    if len(sys.argv) > 1:
        print 'get param'
        # delete_file_folder(sys.argv[1])
    else :
        delete_file_folder(get_current_dir())
        print "delete finished!"


raw_input() 
    