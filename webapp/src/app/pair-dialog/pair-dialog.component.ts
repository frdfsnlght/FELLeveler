import { Component, OnInit, OnDestroy } from '@angular/core';
import { MatDialogRef } from '@angular/material/dialog';
import { Subscription } from 'rxjs';
import { ModelService } from '../model.service';

@Component({
  selector: 'app-pair-dialog',
  templateUrl: './pair-dialog.component.html',
  styleUrls: ['./pair-dialog.component.css']
})
export class PairDialogComponent implements OnInit, OnDestroy {

  mode: string = '';
  btPairedSubscription!: Subscription;
  scanning = false;

  constructor(
    private dialogRef: MatDialogRef<PairDialogComponent>,
    public model: ModelService) {
      this.mode = model.mode.value;
  }

  ngOnInit(): void {
    if (this.mode == 'Tractor') {
      this.scan();
    }
    if (this.mode == 'Implement') {
      this.model.startPairing().subscribe({
        next: (res: string) => {
          console.info('startPairing: ' + res);
        }
      });
      this.btPairedSubscription = this.model.btPaired.asObservable().subscribe(b => {
        if (b)
          this.dialogRef.close(true);
      });
    }
  }

  ngOnDestroy() {
    if (this.mode == 'Implement') {
      this.btPairedSubscription.unsubscribe();
    }
  }

  scan(): void {
    this.scanning = true;
    this.model.scanBTDevices().subscribe({
      next: (res: string) => {
        console.info('scanBTDevices: ' + res);
      },
      complete: () => {
        this.scanning = false;
      }
    });
  }

  pair(address: string): void {
    this.model.pairBTDevice(address).subscribe({
      next: (res: string) => {
        console.info('pairBTDevice: ' + res);
        if (res == 'OK')
          this.dialogRef.close(true);
      }
    });
  }

  cancel(): void {
    if (this.mode == 'Tractor') {
      this.dialogRef.close(false);
    }
    if (this.mode == 'Implement') {
      this.model.stopPairing().subscribe({
        next: (res: string) => {
          console.info('stopPairing: ' + res);
          this.dialogRef.close(false);
        }
      });
    }
  }

}
