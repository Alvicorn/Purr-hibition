import base64
import logging
from queue import Queue
from typing import Optional

import aiohttp
import asyncio
import cv2
import numpy as np


log = logging.getLogger()


class VideoStreamer:
    def __init__(
        self, server_ip: str, stop_condition: asyncio.Event, reconnect_timeout: int = 10
    ):
        self.server_ip = server_ip
        self.stop_condition = stop_condition
        self.reconnect_timeout: int = reconnect_timeout
        self.ws: Optional[aiohttp.ClientWebSocketResponse] = None

    async def stream_video(self, queue: Queue[np.ndarray]):
        """
        Attempt to stream the device's camera frames to a server.

        Args:
            queue (Queue[np.ndarray]): Queue of frames to be sent to a server.
        """
        while not self.stop_condition.is_set():
            async with aiohttp.ClientSession() as session:
                try:
                    log.info(f"Attempting to stream video to {self.server_ip}")
                    self.ws = await session.ws_connect(self.server_ip)
                    log.info(f"Successful connection to {self.server_ip}")

                    while not self.stop_condition.is_set():
                        if queue.empty():
                            await asyncio.sleep(0.01)
                            continue

                        frame = queue.get_nowait()
                        _, buffer = cv2.imencode(".jpg", frame)
                        frame_data = base64.b64encode(buffer).decode("utf-8")
                        await self.ws.send_str(frame_data)

                except aiohttp.ClientConnectorError as e:
                    log.warning(
                        f"Connection failed: {e}. Attempting to "
                        f"connect again in {self.reconnect_timeout} seconds"
                    )
                    await asyncio.sleep(self.reconnect_timeout)
                except Exception as e:
                    log.warning(f"Disconnected to server: {e}")
                    await asyncio.sleep(self.reconnect_timeout)
                finally:
                    if self.ws:
                        await self.ws.close()
