import argparse
import cv2
import logging
import os
import sys
import threading
import time
from queue import Queue, Empty

logger = logging.getLogger()
logger.setLevel(logging.INFO)
formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')

file_handler = logging.FileHandler('log/app.log')
file_handler.setFormatter(formatter)
logger.addHandler(file_handler)

stream_handler = logging.StreamHandler(sys.stdout)
stream_handler.setFormatter(formatter)
logger.addHandler(stream_handler)

class Sensor:
    def get(self):
        raise NotImplementedError("Subclasses must implement method get()")

class SensorX(Sensor):
    def __init__(self, delay: float):
        self._delay = delay
        self._data = 0

    def get(self) -> int:
        time.sleep(self._delay)
        self._data += 1
        return self._data

class SensorCam(Sensor):
    def __init__(self, cam_index, resolution):
        self.cap = cv2.VideoCapture(cam_index, cv2.CAP_AVFOUNDATION)
        if not self.cap.isOpened():
            try:
                self.cap = cv2.VideoCapture(cam_index)
                if not self.cap.isOpened():
                    raise RuntimeError(f"Camera {cam_index} not found")
            except:
                raise RuntimeError(f"Camera {cam_index} not found")
        
        self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, resolution[0])
        self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, resolution[1])
        self._last_frame = None

    def get(self):
        ret, frame = self.cap.read()
        if not ret:
            self._error_count += 1
            if self._error_count > 1:
                logging.error("Camera disconnected or failed")
                raise RuntimeError("Camera disconnected")
            logging.warning("Failed to read frame from camera")
            return None
        self._error_count = 0 
        self._last_frame = frame
        return self._last_frame

    def __del__(self):
        if hasattr(self, 'cap') and self.cap.isOpened():
            self.cap.release()

class WindowImage:
    def __init__(self, freq):
        self.window_name = "Sensor Display"
        self.freq = freq
        self.interval = max(1, int(1000 / self.freq))
        cv2.namedWindow(self.window_name, cv2.WINDOW_NORMAL)

    def show(self, img):
        if img is not None:
            cv2.imshow(self.window_name, img)
        return cv2.waitKey(self.interval)

    def __del__(self):
        cv2.destroyWindow(self.window_name)

def sensor_thread(sensor, queue, stop_event, max_queue_size=1):
    while not stop_event.is_set():
        try:
            data = sensor.get()
            if data is not None:
                while queue.qsize() >= max_queue_size:
                    try:
                        queue.get_nowait()
                    except Empty:
                        break
                queue.put(data)
        except RuntimeError as e:
            logging.error(f"Sensor error: {e}")
            stop_event.set() 
            break
        except Exception as e:
            logging.error(f"Unexpected sensor error: {e}")
            stop_event.set()
            break

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--camera', type=int, default=0, help='Camera index (0, 1, etc.)')
    parser.add_argument('--resolution', type=str, default="640x480", help='Desired resolution (e.g., 640x480)')
    parser.add_argument('--freq', type=int, default=30, help='Display frequency (Hz)')
    args = parser.parse_args()

    try:
        width, height = map(int, args.resolution.split('x'))
    except:
        logging.error("Invalid resolution format")
        return

    if not os.path.exists('log'):
        os.makedirs('log')

    try:
        cam = SensorCam(args.camera, (width, height))
    except Exception as e:
        logging.error(f"Camera init failed: {e}")
        return

    sensors = {
        's0': SensorX(0.01),
        's1': SensorX(0.1),
        's2': SensorX(1.0),
        'cam': cam
    }

    queues = {
        's0': Queue(maxsize=1),
        's1': Queue(maxsize=1),
        's2': Queue(maxsize=1),
        'cam': Queue(maxsize=1)
    }

    stop_event = threading.Event()
    threads = []

    for name in sensors:
        t = threading.Thread(target=sensor_thread, 
                           args=(sensors[name], queues[name], stop_event), daemon=True)
        t.start()
        threads.append(t)

    window = WindowImage(args.freq)
    last_frame = None
    sensor_data = {'s0': 0, 's1': 0, 's2': 0}

    try:
        interval = max(1, int(1000 / args.freq))
        while not stop_event.is_set():
            #start_time = time.time()
            for name in ['s0', 's1', 's2']:
                try:
                    sensor_data[name] = queues[name].get_nowait()
                except Empty:
                    pass

            try:
                last_frame = queues['cam'].get_nowait()
            except Empty:
                pass

            if last_frame is not None:
                img = last_frame.copy()
                text = f"S0: {sensor_data['s0']}  S1: {sensor_data['s1']}  S2: {sensor_data['s2']}"
                cv2.putText(img, text, (50, img.shape[0]-20), 
                          cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

                key = window.show(img)
                if key == ord('q') or key == 27:
                    stop_event.set()
                    logging.info("Отключаем камеру")
                    break
            
            cv2.waitKey(1)
            
            """elapsed = time.time() - start_time
            target_delay = max(0, (1.0 / args.freq) - elapsed)
            time.sleep(target_delay)"""


    except KeyboardInterrupt:
        logging.warning("Завершение через ctrl+c")
        pass
    finally:
        stop_event.set()
        logging.info("Синхронизируем потоки")
        for t in threads:
            t.join(timeout=1)
        cv2.destroyAllWindows()
        logging.info("Программа завершает своё выполнение")

if __name__ == "__main__":
    main()