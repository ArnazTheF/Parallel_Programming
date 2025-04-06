import cv2
import argparse
import time
import threading
import queue
from ultralytics import YOLO


class VideoProcessor:
    def __init__(self, video_path, output_filename, mode, num_threads=4):
        self.video_path = video_path
        self.output_filename = output_filename
        self.mode = mode
        self.num_threads = num_threads
        self.frames = []
        self.total_frames = 0
        self.width, self.height, self.fps = 640, 480, 60.0

    def read_video(self):
        cap = cv2.VideoCapture(self.video_path)
        self.width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
        self.height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
        self.fps = cap.get(cv2.CAP_PROP_FPS)
        while True:
            ret, frame = cap.read()
            if not ret:
                break
            self.frames.append(frame)
        self.total_frames = len(self.frames)
        cap.release()

    def process_single_threaded(self):
        model = YOLO('yolov8s-pose.pt')
        fourcc = cv2.VideoWriter_fourcc(*'mp4v')
        out = cv2.VideoWriter(self.output_filename, fourcc, self.fps, (self.width, self.height))
        start_time = time.time()
        for frame in self.frames:
            results = model(frame)
            out.write(results[0].plot())
        out.release()
        print(f"Время обработки (однопоточный режим): {time.time() - start_time:.2f} сек")

    def process_multi_threaded(self):
        input_queue = queue.Queue()
        output_queue = queue.Queue()
        workers = []
        for _ in range(self.num_threads):
            worker = Worker(input_queue, output_queue)
            workers.append(worker)
        writer = Writer(output_queue, self.total_frames, self.output_filename, self.width, self.height, self.fps)
        start_time = time.time()
        for worker in workers:
            worker.start()
        writer.start()
        for idx, frame in enumerate(self.frames):
            input_queue.put((idx, frame))
        for _ in range(self.num_threads):
            input_queue.put(None)
        for worker in workers:
            worker.join()
        output_queue.put(None)
        writer.join()
        print(f"Время обработки ({self.num_threads} потоков): {time.time() - start_time:.2f} сек")

class Worker(threading.Thread):
    def __init__(self, input_queue, output_queue):
        super().__init__()
        self.input_queue = input_queue
        self.output_queue = output_queue
        self.model = YOLO('yolov8s-pose.pt')

    def run(self):
        while True:
            item = self.input_queue.get()
            if item is None:
                break
            idx, frame = item
            results = self.model(frame)
            self.output_queue.put((idx, results[0].plot()))

class Writer(threading.Thread):
    def __init__(self, output_queue, total_frames, output_filename, width, height, fps):
        super().__init__()
        self.output_queue = output_queue
        self.total_frames = total_frames
        self.output_filename = output_filename
        self.width = width
        self.height = height
        self.fps = fps
        self.buffer = {}
        self.expected_idx = 0

    def run(self):
        fourcc = cv2.VideoWriter_fourcc(*'mp4v')
        out = cv2.VideoWriter(self.output_filename, fourcc, self.fps, (self.width, self.height))
        while True:
            item = self.output_queue.get()
            if item is None:
                break
            idx, frame = item
            self.buffer[idx] = frame
            while self.expected_idx in self.buffer:
                out.write(self.buffer[self.expected_idx])
                del self.buffer[self.expected_idx]
                self.expected_idx += 1
        while self.expected_idx < self.total_frames:
            if self.expected_idx in self.buffer:
                out.write(self.buffer[self.expected_idx])
                del self.buffer[self.expected_idx]
                self.expected_idx += 1
            else:
                break
        out.release()

def main():
    parser = argparse.ArgumentParser(description='Обработка видео с YOLOv8s-pose')
    parser.add_argument('video_path', help='Путь к входному видео')
    parser.add_argument('mode', choices=['single', 'multi'], help='Режим выполнения (single/multi)')
    parser.add_argument('output_filename', help='Имя выходного файла')
    args = parser.parse_args()

    processor = VideoProcessor(args.video_path, args.output_filename, args.mode, num_threads=4)
    processor.read_video()

    if processor.mode == 'single':
        processor.process_single_threaded()
    else:
        processor.process_multi_threaded()

if __name__ == "__main__":
    main()