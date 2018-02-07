#ifdef CONFIG_HUAWEI_UFS_VENDOR_MODE
/*enter vendor mode*/
#include "ufs_vendor_mode.h"
#include "ufshcd.h"

static int wait_for_ufs_all_complete(struct ufs_hba *hba, int timeout_ms)
{
	msleep(1);
	while (timeout_ms-- > 0) {
		if (!hba->outstanding_reqs){
			return 0;
		}
		udelay(1000);
	}
	pr_err("%s: outstanding req : 0x%lx\n", __func__, hba->outstanding_reqs);
	return -ETIMEDOUT;
}

int ufs_ioctl_vendor_package(struct ufs_hba *hba,
							struct ufs_ioctl_vendor_state_t *ufs_vendor_ioctl_state_p,
							void __user *buffer)
{
	char data_flag;
	int err = 0;

	dev_info(hba->dev, "%s: ioctl vendor cmd buffering\n", __func__);

	if(ufs_vendor_ioctl_state_p->cmd_count > UFS_IOCTL_VENDOR_PACKAGE_COUNT_MAX)
	{
		dev_err(hba->dev, "%s: ioctl vendor cmd over count, %d\n", __func__, ufs_vendor_ioctl_state_p->cmd_count);
		err = -EINVAL;
		goto out;
	}
	/*init vendor_cmd memory*/
	if(!ufs_vendor_ioctl_state_p->vendor_cmd)
	{
		ufs_vendor_ioctl_state_p->vendor_cmd =
			kzalloc( (size_t)(sizeof(struct ufs_vendor_cmd)*UFS_IOCTL_VENDOR_PACKAGE_COUNT_MAX), GFP_KERNEL);
		if(!ufs_vendor_ioctl_state_p->vendor_cmd)
		{
			dev_err(hba->dev, "%s: vendor cmd mem alloc error\n", __func__);
			err = -ENOMEM;
			goto out;
		}
		ufs_vendor_ioctl_state_p->cmd_count = 0;
	}
	/*get user cmd*/
	err = copy_from_user( &(ufs_vendor_ioctl_state_p->vendor_cmd[ufs_vendor_ioctl_state_p->cmd_count].vendor_cdb),
			buffer, UFS_IOCTL_VENDOR_CDB_LEN);
	if(err)
	{
		dev_err(hba->dev, "%s: copy cdb error\n", __func__);
		goto out;
	}

	err = copy_from_user(&data_flag, buffer+UFS_IOCTL_VENDOR_CDB_LEN, 1);
	if(err)
	{
		dev_err(hba->dev, "%s: copy data flag\n", __func__);
		goto out;
	}
	if(data_flag)
	{
		ufs_vendor_ioctl_state_p->vendor_cmd[ufs_vendor_ioctl_state_p->cmd_count].buf =
			kzalloc((size_t)(PAGE_SIZE), GFP_KERNEL);
		if(ufs_vendor_ioctl_state_p->vendor_cmd[ufs_vendor_ioctl_state_p->cmd_count].buf == NULL)
		{
			err = -ENOMEM;
			goto out;
		}
		ufs_vendor_ioctl_state_p->vendor_cmd[ufs_vendor_ioctl_state_p->cmd_count].data_flag = 1;
	}
	ufs_vendor_ioctl_state_p->cmd_count++;

out:
	return err;

}

int ufs_ioctl_vendor_package_tick(struct ufs_hba *hba,
								struct scsi_device *dev,
								struct ufs_ioctl_vendor_state_t *ufs_vendor_ioctl_state_p,
								void __user *buffer)
{
	int err = 0;
	int index = 0;
	struct ufs_vendor_cmd* vendor_cmd;
	char* buf;

	dev_err(hba->dev, "%s: send all vendor cmd\n", __func__);
	/*check if vendor_cmd has already init*/
	if(ufs_vendor_ioctl_state_p->vendor_cmd == NULL)
	{
		dev_err(hba->dev, "%s: vendor package cmd not init\n", __func__);
		err = -EINVAL;
		goto out;
	}

	scsi_block_requests(dev->host);
	err = wait_for_ufs_all_complete(hba, 5000);
	dev_err(hba->dev, "%s: block the queue, ret == %d\n", __func__, err);
	for(index = 0; index < ufs_vendor_ioctl_state_p->cmd_count; index++)
	{
		vendor_cmd = &(ufs_vendor_ioctl_state_p->vendor_cmd[index]);
		if ((vendor_cmd->vendor_cdb[0] == FORMAT_UNIT)
			|| (vendor_cmd->vendor_cdb[0] == READ_6)
			|| (vendor_cmd->vendor_cdb[0] == READ_10)
			|| (vendor_cmd->vendor_cdb[0] == READ_16)
			|| (vendor_cmd->vendor_cdb[0] == UNMAP)
			|| (vendor_cmd->vendor_cdb[0] == WRITE_6)
			|| (vendor_cmd->vendor_cdb[0] == WRITE_10)
			|| (vendor_cmd->vendor_cdb[0] == WRITE_16))
		{
			dev_err(hba->dev, "%s: not allow ioctl this cmd:0x%x\n", __func__, vendor_cmd->vendor_cdb[0]);
			goto error_out;
		}

		buf = NULL;
		if(vendor_cmd->data_flag)
		{
			if(vendor_cmd->buf == NULL)
			{
				pr_err("data bufffer is null\n");
				err = -EINVAL;
				goto error_out;
			}
			buf = vendor_cmd->buf;
		}
		err = ufshcd_send_vendor_scsi_cmd(hba, dev, vendor_cmd->vendor_cdb, (void*)buf);
		if(err)
		{
			dev_err(hba->dev, "%s: send cmd[0x%x] error %d\n", __func__, vendor_cmd->vendor_cdb[0],err);
			goto out;
		}
		if(buf)
		{
			err = copy_to_user( buffer+(unsigned int)(index*UFS_VENDOR_DATA_SIZE_MAX), buf, UFS_VENDOR_DATA_SIZE_MAX);
			if(err){
				dev_err(hba->dev, "%s: copy to user error %d\n", __func__, err);
				goto error_out;
			}
		}
	}

error_out:
	scsi_unblock_requests(dev->host);
	if(ufs_vendor_ioctl_state_p->vendor_cmd)
	{
		for(index = 0; index < ufs_vendor_ioctl_state_p->cmd_count; index++)
		{
			if(ufs_vendor_ioctl_state_p->vendor_cmd[index].data_flag
				&&ufs_vendor_ioctl_state_p->vendor_cmd[index].buf)
			{
				kfree(ufs_vendor_ioctl_state_p->vendor_cmd[index].buf);
				ufs_vendor_ioctl_state_p->vendor_cmd[index].buf = NULL;
			}
		}
		kfree(ufs_vendor_ioctl_state_p->vendor_cmd);
		ufs_vendor_ioctl_state_p->vendor_cmd = NULL;
	}
	ufs_vendor_ioctl_state_p->cmd_count = 0;

out:
	return err;
}
#endif
