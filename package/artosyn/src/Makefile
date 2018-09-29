
cmd_demo = app/cmd_demo
video_demo = app/send_video
upgrade_demo = app/upgrade_demo
#send_video = app/send_video
#uvc_video = app/uvc_video

all:
	cd $(cmd_demo) ; $(MAKE) 
	cd $(video_demo) ; $(MAKE) 
	cd $(upgrade_demo) ; $(MAKE) 
#	cd $(send_video) ; $(MAKE) 
#	cd $(uvc_video) ; $(MAKE) 

clean:
	cd $(cmd_demo) ; $(MAKE) clean
	cd $(video_demo) ; $(MAKE) clean
	cd $(upgrade_demo) ; $(MAKE) clean
#	cd $(send_video) ; $(MAKE) clean
#	cd $(uvc_video) ; $(MAKE) clean