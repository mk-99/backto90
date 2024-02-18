import argparse
import json
from dataclasses import dataclass, asdict, is_dataclass
from faker import Faker
from faker.providers import BaseProvider
from datetime import date, datetime, timedelta


DEFAULT_ZONE = 2

@dataclass
class Fido_address:
    net: int
    node: int
    zone: int = DEFAULT_ZONE
    point: int | None = None

    def __str__(self) -> str:
        node_part = f"{self.zone}:{self.net}/{self.node}"
        point_part = f".{self.point}" if self.point else ""
        return node_part + point_part

@dataclass
class Fido_message:
    src: Fido_address
    dst: Fido_address
    snt_datetime: datetime
    rcv_datetime: datetime
    from_name: str = ""
    to_name: str = ""
    subj: str = ""
    text: str = ""

    def __str__(self) -> str:
        from_str = self.from_name + " " + str(self.src)
        to_str = self.to_name + " " + str(self.dst)

        return f"From: {from_str}\nTo: {to_str}\nSent: {self.snt_datetime}\nReceived: {self.rcv_datetime}\nSubj: {self.subj}\n\n{self.text}"


class FidoAddressProvider(BaseProvider):
    """Faker provider - Fidonet address with optional point"""

    def fido_address(self) -> Fido_address:
        """Gerenrate random address with default zone and no point in most cases"""

        zone = DEFAULT_ZONE if self.random_int(0, 99) < 85 else self.random_int(min=1, max=6)
        net = self.random_int(min=101, max=999)
        node = self.random_int(min=1, max=400)
        point = None if self.random_int(0, 99) < 85 else self.random_int(min=1, max=55)

        return Fido_address(zone=zone, net=net, node=node, point=point)

class FidoMessageProvider(BaseProvider):
    """Faker provider - Fidonet message"""

    def fido_message(self) -> Fido_message:
        """Generate fido message with fake components"""

        fake = Faker()
        Faker.seed()
        fake.add_provider(FidoAddressProvider)
        snt_datetime = fake.date_time_between(
            datetime(year=1987, month=1, day=1),
            datetime(year=2000, month=12, day=31)
        )
        rcv_datetime = fake.date_time_between(
            snt_datetime,
            snt_datetime + timedelta(hours=8)
        )

        return Fido_message(
            src=fake.fido_address(),
            dst=fake.fido_address(),
            from_name=fake.name(),
            to_name=fake.name(),
            snt_datetime=snt_datetime,
            rcv_datetime=rcv_datetime, 
            subj=fake.sentence(nb_words=8, variable_nb_words=True),
            text=fake.text(max_nb_chars=1500)
        )

class Fido_json_encoder(json.JSONEncoder):
    def default(self, o):
        if is_dataclass(o):
            return asdict(o)
        if isinstance(o, datetime):
            return str(o)
        if isinstance(o, date):
            return str(o)
        return super().default(o)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Generate fake Fido messages, in text or in JSON format')
    parser.add_argument("-n", "--number", type=int, required=False, default=50, help="number of messages to generate")
    parser.add_argument("-j", "--json", action="store_true", required=False, help="output in json format (default is text)")
    args = parser.parse_args()

    fake = Faker()
    fake.add_provider(FidoAddressProvider)
    fake.add_provider(FidoMessageProvider)

    if args.json:
        msg_list = []
        for _ in range(args.number):
            msg_list.append(asdict(fake.fido_message()))
        print(json.dumps(msg_list, cls=Fido_json_encoder))
    else:
        for _ in range(args.number):
            print(fake.fido_message())
            print("================")
