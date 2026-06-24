#pragma once

#include <stdint.h>

enum class Gy511Status : uint8_t {
  InitError,
  AccelInitError,
  MagInitError,
  I2cInitError,
  MagOnly,
  AccelReadError,
  MagReadError,
  I2cReadError,
  Ok,
};

inline const char* gy511StatusLabel(Gy511Status status) {
  switch (status) {
    case Gy511Status::InitError:
      return "INIT ERR";
    case Gy511Status::AccelInitError:
      return "ACC INIT";
    case Gy511Status::MagInitError:
      return "MAG INIT";
    case Gy511Status::I2cInitError:
      return "I2C INIT";
    case Gy511Status::MagOnly:
      return "MAG ONLY";
    case Gy511Status::AccelReadError:
      return "ACC ERR";
    case Gy511Status::MagReadError:
      return "MAG ERR";
    case Gy511Status::I2cReadError:
      return "I2C ERR";
    case Gy511Status::Ok:
      return "OK";
  }
  return "READ ERR";
}

inline bool gy511StatusIsInitFailure(Gy511Status status) {
  return status == Gy511Status::InitError ||
         status == Gy511Status::AccelInitError ||
         status == Gy511Status::MagInitError ||
         status == Gy511Status::I2cInitError;
}
