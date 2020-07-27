PROGRAM PlayAndSaveVideo {
    Libraries {
    }
    Handlers {
       
    }
    Renderers {
       
    }
    Sources {
        videoIn from 'E:/video.mp4';
        audioIn from '/tmp/data/in.ogg';
        audioOut from '/tmp/data/out.ogv';
    }
    Sets {
        
    }
    Elements {
	time = '00:00:06';
	num = 5;
    }
    Tuples {
	
    }
    Aggregates {
 	
    }
    Actions {
	
		if(num < 10){
			Download videoBundled From videoIn With OGGIn;

			Timeline As time  {
					Render videoBundled With VideoOutput;
			}
		}
    }
}



