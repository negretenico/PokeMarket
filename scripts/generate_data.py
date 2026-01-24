import struct
import os
import random

MSG_NEW_ORDER = 1
MSG_CANCEL_ORDER = 2

PAYLOAD_FMT_NEW = "<Q B Q I"   # order_id, side, price, qty
PAYLOAD_FMT_CANCEL = "<Q"      # order_id
HEADER_FMT = "<B H"            # msg_type, payload length

HEADER_SIZE = struct.calcsize(HEADER_FMT)

TARGET_BYTES = 1 << 30      # 1 GiB
BATCH_BYTES = 1 << 20       # 1 MiB
CANCEL_RATE = 0.3
MAX_TRACKED_ORDERS = 100_000


def make_new_order(order_id, side, price, quantity):
    payload = struct.pack(PAYLOAD_FMT_NEW, order_id, side, price, quantity)
    header = struct.pack(HEADER_FMT, MSG_NEW_ORDER, len(payload))
    return header + payload


def make_cancel_order(order_id):
    payload = struct.pack(PAYLOAD_FMT_CANCEL, order_id)
    header = struct.pack(HEADER_FMT, MSG_CANCEL_ORDER, len(payload))
    return header + payload


def generate(path="..\\data\\orders.bin"):
    order_id = 1

    # Maintain BOTH structures for O(1) random cancel
    active_set = set()
    active_list = []

    buffer = []
    buffer_bytes = 0
    total_bytes_written = 0

    os.makedirs(os.path.dirname(path), exist_ok=True)

    with open(path, "wb") as f:
        while total_bytes_written < TARGET_BYTES:

            do_cancel = active_list and random.random() < CANCEL_RATE

            if do_cancel:
                # O(1) random cancel
                idx = random.randrange(len(active_list))
                cancel_id = active_list[idx]

                # Remove from set
                active_set.remove(cancel_id)

                # Swap-remove from list
                last = active_list[-1]
                active_list[idx] = last
                active_list.pop()

                rec = make_cancel_order(cancel_id)

            else:
                side = random.choice([0, 1])
                price = random.randint(1, 10_000_000)
                quantity = random.randint(1, 1000)

                rec = make_new_order(order_id, side, price, quantity)

                if len(active_list) < MAX_TRACKED_ORDERS:
                    active_set.add(order_id)
                    active_list.append(order_id)

                order_id += 1

            buffer.append(rec)
            buffer_bytes += len(rec)

            if buffer_bytes >= BATCH_BYTES:
                f.write(b"".join(buffer))
                total_bytes_written += buffer_bytes
                buffer.clear()
                buffer_bytes = 0

                if total_bytes_written % (100 << 20) < BATCH_BYTES:
                    print(
                        f"Written: {total_bytes_written >> 20} MB / "
                        f"{TARGET_BYTES >> 20} MB"
                    )

        if buffer:
            data = b"".join(buffer)
            f.write(data)
            total_bytes_written += len(data)

    final_size = os.path.getsize(path)
    print(f"\nWrote {path} ({final_size} bytes)")
    print(f"Total orders created: {order_id - 1}")


if __name__ == "__main__":
    generate()
