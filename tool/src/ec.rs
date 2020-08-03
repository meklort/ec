use hwio::{Io, Pio};
extern crate std;
use std::fmt;
use std::string::String;

use crate::{
    Error,
    Spi,
    SpiTarget,
    SuperIo,
    Timeout,
    timeout
};

#[derive(Clone, Copy, Debug)]
#[repr(u8)]
pub enum Cmd {
    None = 0,
    Probe = 1,
    Board = 2,
    Version = 3,
    Print = 4,
    Spi = 5,
    Reset = 6,
    FanGet = 7,
    FanSet = 8,

    ConfigGetName = 32,
    ConfigGetDesc = 33,
    ConfigGetValue = 34,
    ConfigSetValue = 35,
    ConfigCompact = 36,
}

pub const CMD_SPI_FLAG_READ: u8 = 1 << 0;
pub const CMD_SPI_FLAG_DISABLE: u8 = 1 << 1;
pub const CMD_SPI_FLAG_SCRATCH: u8 = 1 << 2;
pub const CMD_SPI_FLAG_BACKUP: u8 = 1 << 3;

pub struct Ec<T: Timeout> {
    cmd: u16,
    dbg: u16,
    timeout: T,
}

pub struct Value {
    max: i32,
    min: i32,
    value: i32,
}

impl fmt::Display for Value {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        writeln!(f, "Min: {}", self.min)?;
        writeln!(f, "Max: {}", self.max)?;
        writeln!(f, "Current Value: {}", self.value)?;

        Ok(())
    }
}

impl<T: Timeout> Ec<T> {
    /// Probes for a compatible EC
    pub unsafe fn new(timeout: T) -> Result<Self, Error> {
        let mut sio = SuperIo::new(0x2E);

        let id =
            (sio.read(0x20) as u16) << 8 |
            (sio.read(0x21) as u16);

        match id {
            0x5570 | 0x8587 => (),
            _ => return Err(Error::SuperIoId(id)),
        }

        let mut ec = Ec {
            cmd: 0xE00,
            dbg: 0xF00,
            timeout,
        };

        ec.probe()?;

        Ok(ec)
    }

    /// Read from the command space
    pub unsafe fn read(&mut self, addr: u8) -> u8 {
        Pio::<u8>::new(
            self.cmd + (addr as u16)
        ).read()
    }

    /// Read from the command space
    pub unsafe fn read16(&mut self, addr: u8) -> u16 {
        Pio::<u8>::new(
            self.cmd + (addr as u16)
        ).read() as u16 |
        (Pio::<u8>::new(
            self.cmd + (addr as u16) + 1
        ).read() as u16) << 8

    }

    /// Read from the command space
    pub unsafe fn read32(&mut self, addr: u8) -> u32 {
        Pio::<u8>::new(
            self.cmd + (addr as u16)
        ).read() as u32 |
        (Pio::<u8>::new(
            self.cmd + (addr as u16) + 1
        ).read() as u32) << 8 |
        (Pio::<u8>::new(
            self.cmd + (addr as u16) + 2
        ).read() as u32) << 16 |
        (Pio::<u8>::new(
            self.cmd + (addr as u16) + 3
        ).read() as u32) << 24

    }

    /// Write to the command space
    pub unsafe fn write(&mut self, addr: u8, data: u8) {
        Pio::<u8>::new(
            self.cmd + (addr as u16)
        ).write(data)
    }

    pub unsafe fn write16(&mut self, addr: u8, data: u16) {
        Pio::<u8>::new(
            self.cmd + (addr as u16)
        ).write(data as u8);
        Pio::<u8>::new(
            self.cmd + (addr as u16) + 1
        ).write((data >> 8) as u8)
    }

    pub unsafe fn write32(&mut self, addr: u8, data: u32) {
        Pio::<u8>::new(
            self.cmd + (addr as u16)
        ).write(data as u8);
        Pio::<u8>::new(
            self.cmd + (addr as u16) + 1
        ).write((data >> 8) as u8);
        Pio::<u8>::new(
            self.cmd + (addr as u16) + 2
        ).write((data >> 16) as u8);
        Pio::<u8>::new(
            self.cmd + (addr as u16) + 3
        ).write((data >> 24) as u8)
    }

    /// Read from the debug space
    pub unsafe fn debug(&mut self, addr: u8) -> u8 {
        Pio::<u8>::new(
            self.dbg + (addr as u16)
        ).read()
    }

    /// Returns Ok if a command can be sent
    unsafe fn command_check(&mut self) -> Result<(), Error> {
        if self.read(0) == Cmd::None as u8 {
            Ok(())
        } else {
            Err(Error::WouldBlock)
        }
    }

    /// Wait until a command can be sent
    unsafe fn command_wait(&mut self) -> Result<(), Error> {
        self.timeout.reset();
        timeout!(self.timeout, self.command_check())
    }

    /// Run an EC command
    pub unsafe fn command(&mut self, cmd: Cmd) -> Result<(), Error> {
        // All previous commands should be finished
        self.command_check()?;
        // Write command byte
        self.write(0, cmd as u8);
        // Wait for command to finish
        self.command_wait()?;
        // Read response byte and test for error
        match self.read(1) {
            0 => Ok(()),
            err => Err(Error::Protocol(err)),
        }
    }

    /// Probe for EC
    pub unsafe fn probe(&mut self) -> Result<u8, Error> {
        self.command(Cmd::Probe)?;
        let signature = (
            self.read(2),
            self.read(3)
        );
        if signature == (0x76, 0xEC) {
            let version = self.read(4);
            Ok(version)
        } else {
            Err(Error::Signature(signature))
        }
    }

    /// Read board from EC
    pub unsafe fn board(&mut self, data: &mut [u8]) -> Result<usize, Error> {
        self.command(Cmd::Board)?;
        let mut i = 0;
        while i < data.len() && (i + 2) < 256 {
            data[i] = self.read((i + 2) as u8);
            if data[i] == 0 {
                break;
            }
            i += 1;
        }
        Ok(i)
    }

    /// Read version from EC
    pub unsafe fn version(&mut self, data: &mut [u8]) -> Result<usize, Error> {
        self.command(Cmd::Version)?;
        let mut i = 0;
        while i < data.len() && (i + 2) < 256 {
            data[i] = self.read((i + 2) as u8);
            if data[i] == 0 {
                break;
            }
            i += 1;
        }
        Ok(i)
    }

    pub unsafe fn print(&mut self, data: &[u8]) -> Result<usize, Error> {
        let flags = 0;
        for chunk in data.chunks(256 - 4) {
            for i in 0..chunk.len() {
                self.write(i as u8 + 4, chunk[i]);
            }

            self.write(2, flags);
            self.write(3, chunk.len() as u8);
            self.command(Cmd::Print)?;
            if self.read(3) != chunk.len() as u8 {
                return Err(Error::Verify);
            }
        }
        Ok(data.len())
    }

    pub unsafe fn spi(&mut self, target: SpiTarget, scratch: bool) -> Result<EcSpi<T>, Error> {
        let mut spi = EcSpi {
            ec: self,
            target,
            scratch,
        };
        spi.reset()?;
        Ok(spi)
    }

    pub unsafe fn reset(&mut self) -> Result<(), Error> {
        self.command(Cmd::Reset)
    }

    pub unsafe fn fan_get(&mut self, index: u8) -> Result<u8, Error> {
        self.write(2, index);
        self.command(Cmd::FanGet)?;
        Ok(self.read(3))
    }

    pub unsafe fn fan_set(&mut self, index: u8, duty: u8) -> Result<(), Error> {
        self.write(2, index);
        self.write(3, duty);
        self.command(Cmd::FanSet)
    }

    pub unsafe fn config_get_name(&mut self, index: u8) -> Result<String, Error> {
        self.write(2, index);
        self.command(Cmd::ConfigGetName)?;
        let mut i = 0;
        let mut str = String::from("");

        while (i + 2) < 256 {
            let data = self.read((i + 2) as u8) as char;
            if data == '\0' {
                break;
            }
            i += 1;
            str.push(data);
        }
        Ok(str)
    }

    pub unsafe fn config_get_desc(&mut self, index: u8) -> Result<String, Error> {
        self.write(2, index);
        self.command(Cmd::ConfigGetDesc)?;
        let mut i = 0;
        let mut str = String::from("");

        while (i + 2) < 256 {
            let data = self.read((i + 2) as u8) as char;
            if data == '\0' {
                break;
            }
            i += 1;
            str.push(data);
        }
        Ok(str)
    }

    pub unsafe fn config_get_value(&mut self, index: u8) -> Result<Value, Error> {
        self.write(2, index);
        self.command(Cmd::ConfigGetValue)?;

        let value = Value {
            min: self.read32(2) as i32,
            max: self.read32(6) as i32,
            value: self.read32(10) as i32,
        };

        Ok(value)
    }

    pub unsafe fn config_set_value(&mut self, index: u8, value: i32) -> Result<(), Error> {
        self.write32(3, value as u32);
        self.write(2, index);
        self.command(Cmd::ConfigSetValue)?;

        Ok(())
    }

    pub unsafe fn config_compact(&mut self) -> Result<(), Error> {
        self.command(Cmd::ConfigCompact)?;

        Ok(())
    }

}

pub struct EcSpi<'a, T: Timeout> {
    ec: &'a mut Ec<T>,
    target: SpiTarget,
    scratch: bool,
}

impl<'a, T: Timeout> EcSpi<'a, T> {
    fn flags(&self, read: bool, disable: bool) -> u8 {
        let mut flags = 0;

        if read {
            flags |= CMD_SPI_FLAG_READ;
        }

        if disable {
            flags |= CMD_SPI_FLAG_DISABLE;
        }

        if self.scratch {
            flags |= CMD_SPI_FLAG_SCRATCH;
        }

        match self.target {
            SpiTarget::Main => (),
            SpiTarget::Backup => {
                flags |= CMD_SPI_FLAG_BACKUP;
            },
        }

        flags
    }
}

impl<'a, T: Timeout> Spi for EcSpi<'a, T> {
    fn target(&self) -> SpiTarget {
        self.target
    }

    /// Disable SPI chip, must be done before and after a transaction
    unsafe fn reset(&mut self) -> Result<(), Error> {
        let flags = self.flags(false, true);
        self.ec.write(2, flags);
        self.ec.write(3, 0);
        self.ec.command(Cmd::Spi)?;
        if self.ec.read(3) != 0 {
            return Err(Error::Verify);
        }
        Ok(())
    }

    /// SPI read
    unsafe fn read(&mut self, data: &mut [u8]) -> Result<usize, Error> {
        let flags = self.flags(true, false);
        for chunk in data.chunks_mut(256 - 4) {
            self.ec.write(2, flags);
            self.ec.write(3, chunk.len() as u8);
            self.ec.command(Cmd::Spi)?;
            if self.ec.read(3) != chunk.len() as u8 {
                return Err(Error::Verify);
            }

            for i in 0..chunk.len() {
                chunk[i] = self.ec.read(i as u8 + 4);
            }
        }
        Ok(data.len())
    }

    /// SPI write
    unsafe fn write(&mut self, data: &[u8]) -> Result<usize, Error> {
        let flags = self.flags(false, false);
        for chunk in data.chunks(256 - 4) {
            for i in 0..chunk.len() {
                self.ec.write(i as u8 + 4, chunk[i]);
            }

            self.ec.write(2, flags);
            self.ec.write(3, chunk.len() as u8);
            self.ec.command(Cmd::Spi)?;
            if self.ec.read(3) != chunk.len() as u8 {
                return Err(Error::Verify);
            }
        }
        Ok(data.len())
    }
}

impl<'a, T: Timeout> Drop for EcSpi<'a, T> {
    fn drop(&mut self) {
        unsafe {
            let _ = self.reset();
        }
    }
}
