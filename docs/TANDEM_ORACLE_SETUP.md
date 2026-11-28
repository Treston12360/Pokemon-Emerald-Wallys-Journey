# Project Tandem - setting up the server on Oracle Cloud (step by step)

Brennan 2026-11-27 19:17: "keep all the info about setting up oracle in a list or steps and when i ask about how to
set up oracle again provide me that list so i can follow it at a later date." This is that list - give it back as it
is when he asks. (What the files do and why: docs/TANDEM_HOWTO.md, "A free cloud computer".)

## Before you start
- The game zip with the `cloud-server` folder in it (WallysJourneyServer-linux, setup-server.sh, src/).

## 1. Account - DONE (2026-11-27)
- oracle.com/cloud/free: email, phone, a credit/debit card for identity (not charged unless you upgrade; no prepaid or
  virtual cards). Home region: US Midwest (Chicago) - free computers can only be made there.

## 2. The network - DONE (2026-11-27: "Wallys Journey Network")
- Menu > Networking > Virtual cloud networks > **Start VCN Wizard** > **"Create VCN with Internet Connectivity"**
- Name: Wallys Journey Network. Keep the defaults (10.0.0.0/16, public subnet 10.0.0.0/24, private subnet 10.0.1.0/24).
- Next > Create.

## 3. Open the game's port in Oracle's firewall
- Menu > Networking > Virtual cloud networks > **Wallys Journey Network** > the **public subnet** > its
  **Security List** > **Add Ingress Rules**:
  - Source CIDR: `0.0.0.0/0`
  - IP protocol: TCP
  - Destination port range: `27015`
- Add Ingress Rules.

## 4. Make the computer
Menu > Compute > Instances > **Create instance**
1. **Name:** Wallys Journey. Compartment: your root one.
2. **Placement:** AD 1 (if it says "Out of capacity", try AD 2, then AD 3).
   Advanced options > Capacity type: keep **On-demand capacity** (not Preemptible, reservation, dedicated host).
3. **Image:** Canonical Ubuntu 24.04.
4. **Shape:** **Ampere > VM.Standard.A1.Flex**, **1 OCPU, 6 GB** - it must say **"Always Free-eligible"**; don't raise
   the numbers. (VM.Standard.E2.1.Micro, under "Specialty and previous generation", AD 1 only, is also free if it
   ever shows.) **Never VM.Standard.E5.Flex or other AMD/Intel shapes - those cost money.**
5. **Security:** leave Shielded instance off; no security attributes. Next.
6. **Networking:**
   - Primary network: **Select existing virtual cloud network** > Wallys Journey Network
   - Subnet: **Select existing subnet** > **public subnet-Wallys Journey Network (regional)**
   - Private IPv4: Automatically assign
   - **Automatically assign public IPv4 address: ON** (it unlocks once the public subnet is picked)
   - IPv6: off
   - **Add SSH keys: "Generate a key pair for me" > Download private key** - keep the file (e.g. Documents\oracle).
     Without it you can't get in, and Oracle won't give it again.
7. **Storage:** defaults (46.6 GB boot volume, encryption as it is), no block volumes.
8. **Review:** A1.Flex 1 OCPU / 6 GB, the AD, public IPv4 yes, key downloaded > **Create**.
9. Wait for **Running**; note the **Public IP address** on the instance's page.

If every AD says **"Out of capacity"** (it did on 2026-11-27): try again later (different times of day), or upgrade
to **Pay As You Go** (Billing & Cost Management > Upgrade; free things stay free, anything beyond them is charged -
set a **$1 budget alert** under Billing > Budgets first). Pay As You Go also stops Oracle shutting the computer down
as "idle".

## 5. Put the server on it
Windows PowerShell, in the folder holding `cloud-server` and the key file (KEYFILE = the key's file name,
PUBLIC-IP = from step 4):
```
scp -r -i KEYFILE cloud-server ubuntu@PUBLIC-IP:~
ssh -i KEYFILE ubuntu@PUBLIC-IP
cd cloud-server
sudo bash setup-server.sh
```
On the ARM computer it first builds the server (about a minute). It prints the **invite code** and the address.

## 6. Everyone (you too) joins
In `WallysJourney.cfg` next to the game:
```
tandemJoin=PUBLIC-IP:27015
tandemInvite=THE-CODE      (first time only)
tandemName=YourName
```

## Later
- **New game version:** upload the new `cloud-server` folder (the `scp` line) and run `sudo bash setup-server.sh` in it
  again (players, keys and the code are kept).
- **Who is playing:** on the computer, `sudo journalctl -u wallys-server -f` (Ctrl+C to stop watching).
- **Stopped as idle:** start it again on the instance's page.
