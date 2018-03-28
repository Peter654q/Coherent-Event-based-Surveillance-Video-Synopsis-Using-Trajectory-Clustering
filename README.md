# Coherent-Event-based-Surveillance-Video-Synopsis-Using-Trajectory-Clustering
環境配置
openCV 3.3.0, ubuntu14.04, GNU Make 3.81

1. 先創立資料夾，其名稱和影片名稱相同(請去掉副檔名)
裏面分別創建以下6個子資料夾
(
BG：放置背景圖片(main-opencv.cpp所使用)
obj_n：放置未排序編號之物體圖片(main-opencv.cpp所使用)
obj_t：放置排序後編號之物體圖片(video.cpp所使用)
obj_txt：放置物體各自資訊之txt file(video.cpp所使用)
txt_n：放置未tracking物體之txt file(main-opencv.cpp所使用)
txt_t：放置初步tracling過後之txt file(object_tracking.cpp所使用)
)

2. main-opencv.cpp
範例輸入：./main-opencv ../00004.avi -s -r -t -n -f
	-s:save object images
	-r:show bounding boxes
	-t:save txt files
	-n:open night mode
	-f:modify the frame number(int) of background model training
檔名：FXXX.txt (放在folder/txt_n內)
連接法做完的label
bounding box left
bounding box top 
bounding box width
bounding box height 
連接部份面積
中心點x
中心點y
(若需要執行video.cpp請務必加上-t)

3. video.cpp
(其輸入為kalman_trajectory.txt)
範例輸入：./video ../00004.avi -v -t -s -n -p -d
	-v:save video
	-t:modify transparency
	-s:modify object delay time
	-n:open night mode
	-p:modify the scale factor of shrinking and enlarging
	-d:modify the distance threshold to do the object scheduling
檔名：output2.avi(folder內)

3-1. object_tracking.h
(其輸入為main-opencv.cpp的輸出，和object_tracking.cpp的輸出)
範例輸入：由video.cpp呼叫
檔名：F_outXXX.txt (放在folder/txt_t內)
初步比較的物體編號
bounding box left
bounding box top 
bounding box width
bounding box height 
連接部份面積
中心點x
中心點y

3-2. kalman.h
(其輸入為object_tracking.cpp的輸出)
範例輸入：由video.cpp呼叫
檔名：kalman_trajectory.txt(所有物體軌跡都在裏面，folder內)
-2 (代表後面接framenumber) frame_number
物體編號 預測x 預測y 物體寬 物體高

